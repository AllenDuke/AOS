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
        assert(src>=0); /* 系统任务没有调用mm相关的东西 */
        curr_mp=&mmProcs[src];
        mm_who=src;
        int reply = 1;

        int msgtype = mm_msg.type;

        kprintf("{MM}->get msg from:%d, type:%d  \n",src,msgtype);

        switch (msgtype) {
            case FORK:
                mm_msg.RETVAL = mm_do_fork();
                break;
            case EXIT:
                mm_do_exit();
                reply = 0;
                break;
            case EXEC:
                mm_msg.RETVAL = mm_do_exec();
                break;
            case WAIT:
                mm_do_wait();
                reply = 0;
                break;
            default:
                dump_msg("{MM}->unknown msg: ", &mm_msg);
                assert(0);
                break;
        }

        if (reply) {
            mm_msg.type = SYSCALL_RET;
            send(src, &mm_msg);
            kprintf("{MM}->service done\n");
        }

    }

}

PRIVATE void mm_init() {
    register int proc_nr;
    register MMProcess *rmp;

    /* 初始化内存管理器所有的进程表项 */
    for (proc_nr = 0; proc_nr <= ORIGIN_PROC_NR; proc_nr++) {
        rmp = &mmProcs[proc_nr];
        rmp->flags |= IN_USE;
        /* 拿到该进程的内存映像，它很重要对于MM，这些信息用于FORK。 */
        get_mem_map(proc_nr, &rmp->map);
    }

//    phys_page totalPages = gp_bootParam->memorySize >> 2; /* memorySize的单位是KB */

    phys_page totalPages = (256 * 1024) >> 3; /* todo 这里就假定 实际 是256MB */

    mem_init(0, totalPages);
    phys_page initCost = FREE_BASE >> PAGE_SHIFT; /* 初始时，认为FREE_BASE以下已被内核使用 */
    alloc(initCost); /* 这里不能直接把宏当参数，这样入参会变成0 */

    /* 得到剩余可用的空闲内存，总内存减去程序可以使用的空间即可 */
    phys_page freePages = totalPages - PROC_BASE_PAGE;

    /* 准备ORIGIN进程表项 */
    mmProcs[ORIGIN_PROC_NR].pid = ORIGIN_PID;
    proc_in_use = ORIGIN_PROC_NR + 1;    /* 有多少进程正在使用中？ */

    /* 打印内存信息：内存总量、核心内存的使用和空闲内存情况 */
    kprintf("{MM}->total memory size = %dKB, available = %dKB freePages = %d.\n", totalPages << 2, freePages << 2, freePages);

    in_outbox(&mm_msg, &mm_msg);
}

/**
 *
 * @param proc_nr 回复的进程
 * @param rs 调用结果（通常是OK或错误号）
 */
PUBLIC void set_reply(int proc_nr, int rs) {
    /**
     * 通过调用的结果填写回复消息，以便稍后发送给用户进程。
     * 系统调用有时会填写消息的其他字段。这仅仅适用于主要返
     * 回值以及用于设置“必须发送回复”标志。
     */
    register MMProcess *rmp = &mmProcs[proc_nr];
    rmp->reply_rs1 = rs;
    rmp->flags |= REPLY;    /* 挂起了一个回复，等待处理 */
}
