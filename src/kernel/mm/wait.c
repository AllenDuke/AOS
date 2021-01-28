//
// Created by 杜科 on 2020/12/26.
//
#include "core/kernel.h"

PRIVATE int mm_waitpid(int pid, int options);

PRIVATE void clean_origin_other_children(pid_t theOne);

extern MMProcess mmProcs[];
extern Message mm_msg;

PUBLIC int mm_do_wait(void) {
    /**
     * 如果进程执行了wait()，那么进程A将会被堵塞等待直到所有子进程完成运行（或被终止）。
     * WAIT会完成以下操作：
     *  1 - 遍历进程表，寻找进程A的子进程B，如果找到有僵尸进程。
     *      - 清理掉子进程并回复A使其恢复运行。
     *      - 释放该子进程的进程表条目。
     *  2 - 如果没有找到任何子进程是僵尸。
     *      - 进程A进入等待，设置标志位WAITING位。
     *  3 - 如果进程A没有任何子进程。
     *      - 回复一个错误给进程A（没有子进程你瞎调用干啥）。
     * 当然了，上面这些事情将会在mm_waitpid里实现。
     */

    /**
     * pid：-1，代表等待所有子进程
     * options：0，这个参数wait并没有，所以无所谓，随便给就好，你给个666也行。
     */
    return mm_waitpid(mm_msg.PID, 0);
}

/**
 * 当父进程调用wait或者waitpid时，有以下这些情况：
 * 1. 子进程还没有exit，那么mm将不回复父进程，父进程将阻塞，直至子进程exit。
 * 2. 子进程已经exit，现时已成为僵尸进程，那么父进程可以为子收尸了。
 * @param pid 子进程pid
 * @param options
 * @return 1表示回复父进程，否则不回复
 */
PRIVATE int mm_waitpid(pid_t pid, int options) {
    register MMProcess *proc;
    register MMProcess *pre = &mmProcs[mm_who];

    /* 如果当前origin进程，那么必定是waitpid，必定是pid存在，必定是父子关系，origin先进入waitpid状态 */
    if (mm_who == ORIGIN_PROC_NR) {
        /* 清理所有的僵死进程，然后进入waitpid */
        for (proc = &mmProcs[1]; proc < &mmProcs[NR_PROCS]; proc++) {
            if (proc->ppid == pre->pid && (proc->flags & ZOMBIE)) { /* 找到特定的子进程 */
                /* 子进程已经exit，成为一个僵尸进程 */
                exit_cleanup(proc);                         /* 清理掉这个僵尸进程并答复调用者 */
                pre->aliveChildCount--;                     /* 更新存活子进程数量 */
            }
        }
        curr_mp->flags |= WAITPID_STAT;
        return WHANG;
    }

    /**
     * 遍历所有进程
     * 这里需要注意一下，通过pid参数我们可以判断出进程具体要等什么：
     * 1. pid  >  0：这意味着进程正在等待一个特定子进程，pid是这个进程的进程号
     * 2. pid == -1：这意味着进程等待自己的所有子进程
     */
    if (pid > 0) {
        bool_t found = FALSE;
        for (proc = &mmProcs[1]; proc < &mmProcs[NR_PROCS]; proc++) {
            if (proc->pid == pid) {                         /* 找到特定的子进程 */
                if (proc->ppid != pre->pid) {               /* 两者不是父子关系 */
                    mm_msg.PID = NO_TASK;
                    return 1;                               /* 回复父进程，没有这个子进程 */
                }
                if (proc->flags & ZOMBIE) {                 /* 子进程已经exit，成为一个僵尸进程 */
                    u8_t exitStat = exit_cleanup(proc);     /* 清理掉这个僵尸进程并答复调用者 */
                    pre->aliveChildCount--;                  /* 更新存活子进程数量 */
                    if (mm_msg.STATUS == STATUS_NEED)
                        mm_msg.STATUS = exitStat;           /* 返回子进程的退出状态 */
                    return 1;                               /* 回复父进程，清理成功 */
                }
                found = TRUE;                               /* 子进程还存活 */
                break;
            }
        }
        if (!found) {                                       /* 回复父进程，没有这个子进程 */
            mm_msg.PID = NO_TASK;
            return 1;
        }
        /* 接着往下处理这个存活的子进程 */
    } else {                                                /* if (pid==-1) */
        for (proc = &mmProcs[1]; proc < &mmProcs[NR_PROCS]; proc++) {
            if (proc->ppid == proc_addr(mm_who)->pid) {     /* 找到一个子进程 */
                if (proc->flags & ZOMBIE) {                 /* 子进程已exit，成为僵尸 */
                    exit_cleanup(proc);                     /* 清理掉这个僵尸进程 */
                    pre->aliveChildCount--;                 /* 更新存活子进程数量 */
                }
            }
        }
    }

    if (pre->aliveChildCount == 0) {                        /* 没有子进程，或者已经清理完毕 */
        return 1;
    }

    if (pre->aliveChildCount > 0) {                         /* 还有子进程是存活的，父进程应该等待 */
        if (options & WNOHANG) return 0;                    /* 如果进程表示不需要等待子进程，那么请直接回复结果 */
        if (pid == -1) curr_mp->flags |= WAIT_F;            /* 父进程希望等待子进程完成 */
        else if (mm_msg.STATUS == STATUS_NEED) curr_mp->flags |= WAITPID_STAT;
        else curr_mp->flags |= WAITPID;
        return WHANG;                                       /* 没有回复，让调用者等待 */
    }

    assert(pre->aliveChildCount >= 0);                      /* 理论上不会来到负数 */
}
