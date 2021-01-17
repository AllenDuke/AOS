//
// Created by 杜科 on 2020/12/23.
//
#include "core/kernel.h"

PRIVATE void mm_init();

/**
 * 一棵树管理 256 MB内存，一页8k，共有 32k个叶子节点，树高15，全部节点数为 64k-1个，占用空间约为 768KB
 * todo 修改为动态内存管理机制（当前假定内存为256MB） 一棵树管理 16MB，当一颗树用完时，复制出另外一个树管理接下来的16MB内存
 */
PUBLIC CardNode nodes[NR_TREE_NODE]; //todo 这个若是定义在alloc中会触发GP异常，具体原因暂时搞不懂

Message mm_msg;
u8_t *mmbuf = (u8_t *) 0xA00000; /* 10M~11M用于mm */
const int MMBUF_SIZE = 0x100000;

/* 这里只管理用户进程，但index和process.h中的是相同的 */
PUBLIC MMProcess mmProcs[NR_PROCS];

PUBLIC void mm_task(void) {
    mm_init();
//    u32_t base=alloc(15);
//    mem_dump();
//    free(base,15);
//    mem_dump();
    kprintf("{MM}->mm_task is working...\n");
    while (TRUE) {
        rec(ANY);
        int src = mm_msg.source;
        assert(src >= 0); /* 系统任务没有调用mm相关的东西 */
        curr_mp = &mmProcs[src];
        mm_who = src;
        int reply = 1;

        int msgtype = mm_msg.type;

//        kprintf("{MM}->get msg from:%d, type:%d  \n", src, msgtype);

        switch (msgtype) {
            case FORK:
                mm_msg.PID = mm_do_fork();
                break;
            case EXIT:
                mm_do_exit();
                reply = 0;      /* 进程已经退出，不用回复原进程，考虑回复其父 */
                break;
            case EXEC:
                mm_msg.RETVAL = mm_do_exec();
                break;
            case WAIT:
                reply = mm_do_wait();
                break;
            default:
                dump_msg("{MM}->unknown msg: ", &mm_msg);
                assert(0);
                break;
        }

        if (reply) {
            mm_msg.type = SYSCALL_RET;
            send(curr_mp->pid, &mm_msg);
//            kprintf("{MM}->service done\n");
        }

    }

}

PRIVATE void mm_init() {
    register int proc_nr;
    register MMProcess *rmp;

    /* 准备ORIGIN进程表项 */
    rmp = &mmProcs[ORIGIN_PROC_NR];
    rmp->pid = ORIGIN_PID;
    rmp->ppid = NO_TASK;                /* origin没有父亲 */
    rmp->flags |= IN_USE;
    /* 拿到该进程的内存映像，它很重要对于MM，这些信息用于FORK。 */
    get_mem_map(proc_nr, &rmp->map);
    proc_in_use = ORIGIN_PROC_NR + 1;   /* 有多少进程正在使用中？ */

    for (proc_nr = 1; proc_nr < NR_PROCS; proc_nr++) {
        rmp = &mmProcs[proc_nr];
        rmp->ppid = NO_TASK;
    }

//    phys_page totalPages = gp_bootParam->memorySize >> 2; /* memorySize的单位是KB */

    phys_page totalPages = (256 * 1024) >> 3; /* todo 这里就假定 实际 是256MB */

    mem_init(0, totalPages);
    phys_page initCost = FREE_BASE >> PAGE_SHIFT; /* 初始时，认为FREE_BASE以下已被内核使用 */
    alloc(initCost); /* 这里不能直接把宏当参数，这样入参会变成0 */

    /* 得到剩余可用的空闲内存，总内存减去程序可以使用的空间即可 */
    phys_page freePages = totalPages - PROC_BASE_PAGE;

    /* 打印内存信息：内存总量、核心内存的使用和空闲内存情况 */
    kprintf("{MM}->total memory size = %dKB, available = %dKB freePages = %d.\n", totalPages << 2, freePages << 2,
            freePages);

    in_outbox(&mm_msg, &mm_msg);
}

