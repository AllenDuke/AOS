//
// Created by 杜科 on 2020/12/26.
//
#include "core/kernel.h"

PRIVATE int do_exit();

PRIVATE void exit_cleanup(register MMProcess *exit_proc,MMProcess *wait_parent);

extern MMProcess mmProcs[];

PUBLIC int exit(void){
    /* 这个例程接收EXIT调用，但全部工作都是mm_exit()做的。
     * 这样划分是因为POSIX要求应该实现信号，但我们还没有实
     * 现，当以后信号被支持以后，被信号终止运行的进程也需要
     * 做进程退出工作，但它们给出的参数不同，所以我可以先将
     * 其摆在这里为以后信号的实现提供便利。
     */

    MMProcess *exit_proc=curr_mp;
    int exit_status=m_status;
    MMProcess *wait_parent; /* 可能在等待退出进程完成退出的父进程 */
    register int proc_nr;   /* 退出进程的进程索引号 */

    /* 检查一些问题 */
    proc_nr = exit_proc - &mmProcs[0];
    if(exit_proc == NIL_MMPROC) { /* 空进程，大问题，空进程如何调用的exit()？ */
        panic("a NIL Proc try to exit, proc nr in code", PANIC_ERR_NUM);
    }

    /* 好了，在这我们已经可以确定这个进程是一个正常调用exit()的进程了。
     * 我们现在报告领导（内核）和同事（FS、FLY）该进程退出了。
     * 报告参数：退出进程的进程索引号以及父亲的进程索引号
     */
//    mm_tell_fs(EXIT, proc_nr, exit_proc->parent, 0);

    do_exit(proc_nr, exit_proc->ppid);

    /* 释放退出进程所占的内存 */
    free(exit_proc->map.base >> PAGE_SHIFT, exit_proc->map.size >> PAGE_SHIFT);

    /* 设置退出状态 */
    exit_proc->exit_status = exit_status;

    /* 检查父进程是否在等待子进程退出，如果在等待，请解除父进程的
     * 等待状态，使父进程能继续运行。
     */
    wait_parent = &mmProcs[exit_proc->ppid];
    if(wait_parent->flags & WAITING){       /* 父进程在等待退出进程 */
        /* 退出清理工作，告诉父进程一个子进程已经退出并且进程插槽已经被释放。 */
        exit_cleanup(exit_proc, wait_parent);
    } else {                                /* 父进程并不等待 */
        exit_proc->flags = IN_USE | ZOMBIE; /* 僵尸进程 */
    }

    /* 寻找进程表，如果退出进程还存在子进程，那么设置这些子进程的父亲为源进程，随后
     * 完成这些进程的退出工作。
     */
    for(exit_proc = &mmProcs[0]; exit_proc < &mmProcs[NR_PROCS]; exit_proc++){
        if(exit_proc->ppid == proc_nr){   /* 找到了！ */
            exit_proc->ppid = ORIGIN_PROC_NR;
            wait_parent = &mmProcs[ORIGIN_PROC_NR];

            if(wait_parent->flags & WAITING){ /* 源进程正在等待子进程 */
                /* 如果某个子进程进入了僵死状态，做退出清理工作。 */
                if(exit_proc->flags & ZOMBIE) exit_cleanup(exit_proc, wait_parent);
            }
        }
    }

    return ERROR_NO_MESSAGE;    /* 死人不能再讲话 */
}

/**
 *
 * @param exit_proc 退出进程
 * @param exit_status 退出状态
 */
PRIVATE int do_exit(int proc_nr,int pre_proc_nr){
    /* 处理系统调用sys_exit()。
     * 一个进程可以使用一个EXIT调用向内存管理器发送一条
     * 消息来退出一个进程（自己）。内存管理器通过SYS_EXIT通知内核，
     * 最后这项工作由本例程处理。
     * 有一点我需要提前说明：别看平时自己编写的程序退出那么简单，但其
     * 实这个函数可比你想象的要复杂的多。
     */

    register Process *parent, *child;   /* 父子，子进程是要退出的 */
    Process *np, *xp, *search;           /* 用于一会进程队列的遍历 */

    /* 得到子进程（退出）的实例 */
    child = proc_addr(proc_nr);
    if(!is_user_proc(child)) panic("退出进程不是用户进程\n",PANIC_ERR_NUM);    /* 退出进程必须是用户进程 */
    parent = proc_addr(pre_proc_nr);
    if(!is_user_proc(parent)) panic("退出进程的父进程不是用户进程\n",PANIC_ERR_NUM);   /* 父进程也必须是 */

    /* 将退出进的时间记账信息算在父亲头上 */
    interrupt_lock();
    parent->userTime = child->userTime + child->childUserTime;
    parent->sysTime = child->sysTime + child->childSysTime;
    interrupt_unlock();

    /* 关闭退出进程的闹钟 */
    child->alarm = 0;
    /* 如果退出进程处于运行态，堵塞它 */
    if(child->flags == 0) lock_unready(child);

    /* 挂掉的进程将不再有名称 */
    strcpy(child->name, "{none}");

    /* 如果被终止的进程恰巧在队列中试图发送一条消息（即，它可能不是正常手段
     * 退出的，例如被一个信号终止，虽然信号我们还没实现，但这是肯定可能发送
     * 的；又或者内部出了一些奇怪的问题），那么我们必须很小心的从消息队列中
     * 删除它，不能影响整个系统的运行。
     */
    if(child->flags & SENDING){
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
 * @param exit_proc 退出的进程
 * @param wait_parent 上面那个的老爸
 */
PRIVATE void exit_cleanup(register MMProcess *exit_proc,MMProcess *wait_parent){
    /* 完成进程的退出，做一些清理等善后的工作
     * 当一个进程已经结束运行并且它的父进程在等待它的时候，不管这
     * 些事件发生的次序如何，本例程都将被调用执行完成最后的操作。
     * 这些操作包括：
     *  - 解除父进程的等待状态
     *  - 发送一条消息给父进程使其重新运行
     *  - 释放退出进程的进程槽位
     *  - 归还
     */
    int exit_status;

    /* 父进程解除等待状态 */
    wait_parent->flags &= ~WAITING;

    /* 唤醒父进程 */
    exit_status = exit_proc->exit_status;
    wait_parent->reply_rs2 = exit_status;
    set_reply(exit_proc->ppid, exit_proc->pid);

    /* 释放进程槽位，减少计数 */
    exit_proc->flags = 0;
    proc_in_use--;
}