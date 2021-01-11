//
// Created by 杜科 on 2020/12/26.
//

#include <errno.h>
#include <core/global.h>
#include <core/config.h>
#include "stdio.h"
PRIVATE int mm_waitpid(int pid, int options);

extern MMProcess mmProcs[];

PUBLIC int wait(void){
    /* 如果进程执行了wait()，那么进程A将会被堵塞等待直到一个子进程完成运行（或被终止）。
     * WAIT会完成以下操作：
     *  1 - 遍历进程表，寻找进程A的子进程B，如果找到有僵尸进程。
     *      - 清理掉子进程并回复A使其恢复运行。
     *      - 释放该子进程的进程表条目。
     *  2 - 如果没有找到任何子进程是僵尸
     *      - 进程A进入等待，设置标志位WAITING位。
     *  3 - 如果进程A没有任何子进程
     *      - 回复一个错误给进程A（没有子进程你瞎调用干啥）。
     * 当然了，上面这些事情将会在mm_waitpid里实现。
     */

    /* pid：-1，代表等待所有子进程
     * options：0，这个参数wait并没有，所以无所谓，随便给就好，你给个666也行。
     */
    return mm_waitpid(-1, 0);
}

PUBLIC int waitpid(void){
    /* 它和wait的区别就是本调用可以可以非常精准的等待某一个
     * 子进程，而不是所有的。同时，waitpid还允许设置等待选
     * 项，如果等待的子进程还在运行，也可以不堵塞自己，继续
     * 运行。
     */
    return mm_waitpid(m_pid, m_sig_nr);
}

PRIVATE int mm_waitpid(int pid, int options){
    /* 本例程实现waitpid的功能，将其拿出来是因为wait和waitpid本质上没有什么
     * 区别，可以这么说，wait只是waitpid的一个子集，放在这，可以服用这些可以
     * 重复的代码。
     */

    register MMProcess *proc;
    int child_count = 0;        /* 记录子进程数量和 */

    /* 遍历所有进程
     * 这里需要注意一下，通过pid参数我们可以判断出进程具体要等什么：
     *  - pid  >  0：这意味着进程正在等待一个特定进程，pid是这个进程的进程号
     *  - pid == -1：这意味着进程等待自己的任何一个子进程
     *  - pid  < -1：这意味着进程等待一个进程组里的进程
     */
    for(proc = &mmProcs[0]; proc < &mmProcs[NR_PROCS]; proc++){
        if(proc->ppid == mm_who){             /* 是调用者的子进程吗？ */
            if(pid > 0 && pid != proc->pid)continue;
            /* 只有 pid == -1 的情况才能到下面 */
            child_count++;                      /* 记录调用者的子进程数量 */
            if(proc->flags & ZOMBIE){           /* 找到了一个僵尸进程 */
                exit_cleanup(proc, curr_mp);    /* 清理掉这个僵尸进程并答复调用者 */
                return ERROR_NO_MESSAGE;        /* 已经设置了回复，所以不需要回复了 */
            }
        }
    }

    if(child_count > 0){                        /* 没有找到任何子进程是僵尸 */
        if(options & WNOHANG) return 0;         /* 如果进程表示不需要等待子进程，那么请直接回复结果 */
        curr_mp->flags |= WAITING;              /* 父进程希望等待子进程完成 */
        curr_mp->wait_pid = (pid_t)pid;         /* 保存起来等待的进程号 */
        return ERROR_NO_MESSAGE;                /* 没有回复，让调用者等待 */
    } else {
        return ECHILD;                          /* 没有任何子进程 */
    }
}
