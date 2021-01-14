//
// Created by 杜科 on 2020/12/25.
//

#include "core/kernel.h"

PRIVATE pid_t nextPid = ORIGIN_PID + 1;

PRIVATE int do_fork(int child_nr, int pre_nr, int pid);

extern MMProcess mmProcs[];

/**
 * 当一个进程想要创建一个新进程的时候，使用FORK调用生成一个新的进程分支，
 * 这个新进程是调用进程的子进程。
 */
PUBLIC int mm_do_fork(void) {
    register MMProcess *parent;     /* 指向父进程，即调用者 */
    register MMProcess *child;      /* 指向fork出来的子进程 */
    MMProcess *search;
    int child_nr;
    bool_t pid_in_use;
    phys_addr child_base;

    /**
     * 如果在FORK的时候，进程表已经满了，那么我们就没必要开始了，
     * 避免一些不必要的麻烦，因为如果失败了，恢复过程麻烦的你不敢
     * 想象，而且非常容易出错。
     */
    parent = curr_mp;
    if (proc_in_use == NR_PROCS) return EAGAIN;

    /**
     * 子进程需要继承父进程的：正文段、数据段和堆栈段。
     * 我们先进行分配内存，分配大小是父进程的内存大小。
     * 先进行内存分配是为了防止内存不足导致失败可以没
     * 有任何后果，因为新的进程表项还没有进行分配。
     */
    child_base = alloc(parent->map.size >> PAGE_SHIFT) << PAGE_SHIFT;
    if (child_base == NO_MEM) return ENOMEM;     /* 空间分配失败... */

    phys_copy(parent->map.base, child_base, parent->map.size);

    /* 现在，我们必须找到一个空的进程插槽给子进程 */
    for (child = &mmProcs[0]; child < &mmProcs[NR_PROCS]; child++) {
        if (!(child->flags & IN_USE)) {
            /* 找到了，停止寻找 */
            break;
        }
    }

    /* 得到子进程索引号 */
    child_nr = child - &mmProcs[0];  /* 得到子进程的进程索引号 */
    proc_in_use++;     /* 一个新的进程被使用了 */
    /* 设置子进程信息及其内存映像，子进程继承父进程的结构（MM中的） */
    *child = *parent;
    child->ppid = mm_who;     /* 不要忘了父亲是谁 */
    child->flags &= IN_USE;     /* 这个进程插槽已经被使用了，这很重要。 */

    /* 为子进程找到一个可用的进程号，并将其放入进程表中 */
    do {
        pid_in_use = FALSE;
        /* 理论上可能发生以下的问题：
         * 在把一个进程号，例如17，赋值给了一个非常长寿的进程之后，可能会有将近很多很多
         * 的进程被它创建和撤销，我就认为可能有50000个吧，nextPid可能又再次回到回到17。
         * 指定一个我们仍然在使用的进程号是一场灾难（想想，随后某个进程向进程17发送信号的
         * 情形，有多少个进程会收到这个信号呢？）所以我们需要搜索整个进程表以确定被指定的
         * 进程号有没有被使用。虽然有点耗费时间，但是还好的是进程表并不大，只有32个用户进
         * 程能同时存在。
         */
        if (nextPid >= 50000) nextPid = ORIGIN_PID + 1;
        for (search = &mmProcs[0]; search < &mmProcs[NR_PROCS]; search++) {
            if (search->pid == nextPid) {
                pid_in_use = TRUE;
                break;  /* 结束这次查找 */
            }
        }
        child->pid = nextPid;      /* 我们分配给子进程。 */
        nextPid++;
    } while (pid_in_use == TRUE);   /* 找到的进程号依然被使用，就继续 */

    /* 更新子进程的内存映像，子进程的正文段，数据段和堆栈段基址必须引用新分配的 */
    child->map.base = child_base;
    child->exit_status = 0;     /* 退出状态置位 */

    /* 现在告诉内核，一个新进程出现了，内核将会更新内核中的进程信息，比如栈帧信息 */
    do_fork(child_nr, mm_who, child->pid);

    /* tell FS, see fs_fork() */
    Message msg2fs;
    msg2fs.type = FORK;
    msg2fs.PID = child->pid;
    send_rec(FS_TASK, &msg2fs);

    /**
     * 万事具备，就差最后一步，新进程还需要设置自己的内存映像，
     * 它虽然继承自父进程，但它应该有自己的LDT。但是MM自己做不
     * 到，所以需我们需要通知内核来完成。
     */
    new_mem_map(child_nr, mm_who, &child->map);

    /* 子进程的生日！我们给它发一条消息唤醒它。 */
    set_reply(child_nr, 0);

    /* 返回子进程的进程号 */
    return child->pid;
}


PRIVATE int do_fork(int child_nr, int pre_nr, int pid) {
    /* msg_ptr->PROC_NR1是新创建的进程，它的父进程在msg_ptr->PROC_NR2中，
     * msg_ptr->PID是新进程的进程号。
     */

    register Process *child;
    reg_t old_ldt_sel;
    Process *parent;

    child = proc_addr(child_nr);   /* 得到子进程 */
    if (!is_empty_proc(child)) panic("子进程不是空进程\n", PANIC_ERR_NUM);   /* 子进程一定要是一个空进程 */
    parent = proc_addr(pre_nr);  /* 父进程 */
    if (!is_user_proc(parent)) panic("父进程不是用户进程\n", PANIC_ERR_NUM);   /* 父进程必须是一个用户进程 */

    /* 将父进程拷贝给子进程（所有） */
    old_ldt_sel = child->ldtSelector;  /* 防止LDT选择子被覆盖，我们备份它 */
    *child = *parent;                   /* 拷贝进程结构体 */
    child->ldtSelector = old_ldt_sel;  /* 恢复LDT选择子 */
    /* 设置子进程一些独有的信息 */
    child->logicIndex = child_nr;      /* 子进程要记住自己的索引号 */
    child->flags |= NO_MAP;             /* 禁止子进程运行，因为它刚刚出生 */
    child->flags &= ~(PENDING | SIG_PENDING | PROC_STOP);   /* 复位标志，它们不应该继承父进程的这些状态 */
    child->pid = pid;          /* 记住自己的进程号 */
    child->regs.eax = 0;                /* 子进程看到ax是0.就知道自己是fork出来的了。 */
    kprintf(child->name, "%s_fk_%d", parent->name, child->pid);  /* 还是给它起个默认名字吧：父名_fk_子号 */

    /* 清零子进程的时间记账信息 */
    child->userTime = child->sysTime = child->childUserTime = child->childSysTime = 0;

    return OK;  /* OK了 */
}

