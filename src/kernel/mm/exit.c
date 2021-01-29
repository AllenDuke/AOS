//
// Created by 杜科 on 2020/12/26.
//
#include "core/kernel.h"

PRIVATE int do_exit();

PRIVATE void check_pre_wakeup(int preLogicIndex);

extern MMProcess mmProcs[];
extern Message mm_msg;

/**
 * 在unix系统中，一个进程结束了，但是其父进程没有等待(调用wait/waitpid)它，
 * 那么它将变成一个僵尸进程（虽然占用的内存已经释放，但仍在进程表中占用一个位置）。
 *
 * 实际上，在进程exit后，我们也可以做到释放在进程表中的位置的，但是我们位于与unix保持同步，
 * 然后采用unix这种机制。
 *
 * 当子进程执行完后，调用exit退出，会发生以下这这些情况：
 * 1. 父进程还没有调用wait或者waitpid，那么子进程成为僵尸进程。
 * 2. 父进程已经调用了wait或者waitpid，那么进行完整的清理工作，考虑唤醒父进程。
 * @return
 */
PUBLIC int mm_do_exit(void) {
//    kprintf("{MM}->%d want to exit.\n",mm_who);
    /**
     * 这个例程接收EXIT调用，但全部工作都是mm_exit()做的。
     * 这样划分是因为POSIX要求应该实现信号，但我们还没有实
     * 现，当以后信号被支持以后，被信号终止运行的进程也需要
     * 做进程退出工作，但它们给出的参数不同，所以我可以先将
     * 其摆在这里为以后信号的实现提供便利。
     */

    MMProcess *exit_proc = curr_mp;
    int exit_status = mm_msg.STATUS;
    MMProcess *wait_parent;             /* 可能在等待退出进程完成退出的父进程 */
    register int proc_nr;               /* 退出进程的进程索引号 */

    /* 检查一些问题 */
    proc_nr = exit_proc - &mmProcs[0];
    if (exit_proc == NIL_MMPROC) {       /* 空进程，大问题，空进程如何调用的exit()？ */
        panic("a NIL Proc try to exit, proc nr in code", proc_nr);
    }

    int preLogicIndex = get_logicI(exit_proc->ppid);
    if (!is_ok_src_dest(preLogicIndex)) {
        kprintf("{MM}->get a invalid pre addr logicIndex:%d.\n", preLogicIndex);
        assert(0);
    }

    do_exit(proc_nr, preLogicIndex);

    /* 通知fs，不使用mm_msg是因为mm_msg后续还要用 */
    Message msg2fs;
    msg2fs.type = EXIT;
    msg2fs.LOGIC_I = proc_nr;
    send_rec(FS_TASK, &msg2fs);

    /* 释放退出进程所占的内存 */
    free(exit_proc->map.base >> PAGE_SHIFT, exit_proc->map.size >> PAGE_SHIFT);

    /* 设置退出状态 */
    exit_proc->exit_status = exit_status;

    /**
     * 检查父进程是否在等待子进程退出，如果在等待，考虑解除父进程的
     * 等待状态，使父进程能继续运行。
     *
     */
    wait_parent = &mmProcs[preLogicIndex];
    if (wait_parent->flags & WAITING) {         /* 父进程在等待退出进程 */
        u8_t exitStat = exit_cleanup(exit_proc);/* 父亲已进入等待状态，子进程错过了收尸时间，自觉进行自我清理 */
        wait_parent->aliveChildCount--;          /* 更新父进程存活的子进程数量 */
        if (wait_parent->flags & WAITPID) {     /* 如果waitpid */
            wait_parent->flags &= ~WAITING;     /* 直接解除父进程等待状态 */
            mm_msg.type = SYSCALL_RET;
            send(wait_parent->pid, &mm_msg);    /* 发送信息给退出进程的父进程 */
        } else if (wait_parent->flags & WAITPID_STAT) {
            wait_parent->flags &= ~WAITING;     /* 直接解除父进程等待状态 */
            mm_msg.type = SYSCALL_RET;
            mm_msg.STATUS = exitStat;
            send(wait_parent->pid, &mm_msg);    /* 发送信息给退出进程的父进程 */
        } else check_pre_wakeup(preLogicIndex); /* 尝试唤醒父亲 */
    } else {                                    /* 父进程并不等待 */
        exit_proc->flags = IN_USE | ZOMBIE;     /* 成为僵尸进程，等着被收尸 */
    }

    /**
     * 寻找mm进程表，如果退出进程还存在子进程，那么设置这些子进程的父亲为起源进程，
     * 如果发现某个子进程已经exit，那么完成这些进程的退出工作。
     */
    MMProcess *origin = &mmProcs[ORIGIN_PROC_NR];
    MMProcess *tmp;
    for (tmp = &mmProcs[1]; tmp < &mmProcs[NR_PROCS]; tmp++) {
        if (tmp->ppid == exit_proc->pid) {          /* 空闲的进程的ppid为NO_TASK */
            tmp->ppid = ORIGIN_PID;
            origin->aliveChildCount++;                      /* origin白捡一儿子 */
//            kprintf("origin get son:%d.\n", tmp->pid);

            if (origin->flags & WAITPID) {                  /* origin正在等待子进程 */
                /* 刚得一儿子，却发现已经死了，含泪收尸。 */
                if (exit_proc->flags & ZOMBIE) {
                    exit_cleanup(tmp);
                    origin->aliveChildCount--;                   /* 更新父进程存活的子进程数量 */
//                    check_pre_wakeup(ORIGIN_PROC_NR);       /* 如果可以，就唤醒父亲 */
                } else kprintf("new son is not dead.\n");
            }
        }
    }

//    kprintf("{MM}->do exit done\n");
    return ERROR_NO_MESSAGE;
}

/**
 * 清理procs部分
 * @param proc_nr 子进程逻辑索引
 * @param pre_proc_nr 父进程逻辑索引
 * @return
 */
PRIVATE int do_exit(int proc_nr, int pre_proc_nr) {

    register Process *parent, *child;   /* 父子，子进程是要退出的 */

    /* 得到子进程（退出）的实例 */
    child = proc_addr(proc_nr);
    if (!is_user_proc(child)) panic("the proc want to exit is not a user proc\n", proc_nr);
    parent = proc_addr(pre_proc_nr);
    if (!is_user_proc(parent)) panic("the proc want to exit which father is not a user proc\n", pre_proc_nr);

    /* 将退出进的时间记账信息算在父亲头上 */
    interrupt_lock();
    parent->userTime = child->userTime + child->childUserTime;
    parent->sysTime = child->sysTime + child->childSysTime;
    interrupt_unlock();

    /* 关闭退出进程的闹钟 */
    child->alarm = 0;
    /* 如果退出进程处于运行态，堵塞它 */
    if (child->flags == 0) lock_unready(child);

    /* 挂掉的进程将不再有名称 */
    strcpy(child->name, "none");

    /**
     * 如果被终止的进程恰巧在队列中试图发送一条消息（即，它可能不是正常手段
     * 退出的，例如被一个信号终止，虽然信号我们还没实现，但这是肯定可能发送
     * 的；又或者内部出了一些奇怪的问题），那么我们必须很小心的从消息队列中
     * 删除它，不能影响整个系统的运行。
     */
    if (child->flags & SENDING) {
        /* 检查所有进程，看下退出进程是否有在某个消息发送链上 */
        rm_proc_from_waiters(child);
    }

    /* 重置进程的标志和权限 */
    child->flags = 0;
    child->priority = PROC_PRI_NONE;

    return OK;
}

/**
 *
 * @param exit_proc 退出进程
 * @param wait_parent 退出进程的父进程
 * @return 退出进程的退出状态
 */
PUBLIC u8_t exit_cleanup(register MMProcess *exit_proc) {
    /**
     * 完成进程的退出，做一些清理等善后的工作
     * 当一个进程已经结束运行并且它的父进程在等待它的时候，不管这
     * 些事件发生的次序如何，本例程都将被调用执行完成最后的操作。
     * 这些操作包括：
     *  - 释放退出进程的进程槽位
     *  - 归还
     */

    /* 释放进程槽位，减少计数 */
    exit_proc->flags = 0;           /* 重置状态 */
    exit_proc->ppid = NO_TASK;        /* 重置父亲为NO_TASK */
    proc_in_use--;

    return exit_proc->exit_status;
}

/* 检查WAIT_F状态的父进程 */
PRIVATE void check_pre_wakeup(int preLogicIndex) {
    MMProcess *wait_parent = &mmProcs[preLogicIndex];
//    kprintf("mm check pre:%d, child alive:%d.\n",preLogicIndex,wait_parent->aliveChildCount);
    if (wait_parent->aliveChildCount > 0) return;
    if (wait_parent->aliveChildCount == 0) {
        wait_parent->flags &= ~WAITING;                         /* 父进程解除等待状态 */
        mm_msg.type = SYSCALL_RET;
        send(wait_parent->pid, &mm_msg);                        /* 发送信息给退出进程的父进程 */
    }

    assert(wait_parent->aliveChildCount >= 0);                  /* 理论上不会来到负数 */
}