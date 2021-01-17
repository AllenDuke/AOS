/**
 * 进程调度相关
 */
#include "core/kernel.h"

/**
 * 进程正在切换则为TRUE，否则为FALSE；当为TRUE应该禁止硬件中断的产生，不然会
 * 产生一些严重的问题。
 */
PRIVATE bool_t switching = FALSE;

PUBLIC const u8_t intMsgsCapacity = 100;
PUBLIC IntMsg intMsgs[100]; /* 存放即将在interrupt中忽略的int信息 */
PUBLIC u8_t intMsgsSize = 0;

/* 本地函数声明 */
FORWARD void hunter(void);

FORWARD void schedule(void);

/* 就绪一个进程 */
PUBLIC void ready(register Process *p_proc) {
    if (is_task_proc(p_proc)) {
        if (gp_readyHeads[TASK_QUEUE] != NIL_PROC) {
            /* 就绪队列非空，挂到队尾 */
            gp_readyTails[TASK_QUEUE]->p_nextReady = p_proc;
        } else {
            /**
             * IDLE与HARDWARE不参与ready。
             * 若当前处于IDLE，发生中断时进行ready某个任务A, 中断结束后可直接切换到任务A。
             */

            /* 就绪队列是空的，那么这个进程直接就可以运行，并挂在就绪队列头上 */
            gp_curProc = gp_readyHeads[TASK_QUEUE] = p_proc;
        }
        // 队尾指针指向新就绪的进程
        gp_readyTails[TASK_QUEUE] = p_proc;      /* 队尾指针 --> 新就绪的进程 */
        p_proc->p_nextReady = NIL_PROC;         /* 新条目没有后继就绪进程 */
        return;
    }
    if (is_serv_proc(p_proc)) {
        /* 同上 */
        if (gp_readyHeads[SERVER_QUEUE] != NIL_PROC) {
            gp_readyTails[SERVER_QUEUE]->p_nextReady = p_proc;
        } else {
            gp_curProc = gp_readyHeads[SERVER_QUEUE] = p_proc;
        }
        gp_readyTails[SERVER_QUEUE] = p_proc;
        p_proc->p_nextReady = NIL_PROC;
        return;
    }
    /**
     * 用户进程的处理稍微有所不同
     * 我们将用户进程添加到队列的最前面。（对于受I/O约束的进程来说更公平一些。）
     */
    if (gp_readyHeads[USER_QUEUE] != NIL_PROC) {
        gp_readyTails[USER_QUEUE] = p_proc;
    }
    p_proc->p_nextReady = gp_readyHeads[USER_QUEUE];
    gp_readyHeads[USER_QUEUE] = p_proc;
}

/* 阻塞一个进程 */
PUBLIC void unready(register Process *p_proc) {
    /**
     * 将一个不再就绪的进程从其队列中删除，即堵塞。
     * 通常它是将队列头部的进程去掉，因为一个进程只有处于运行状态才可被阻塞。
     * unready 在返回之前要一般要调用 hunter。
     */
    register Process *p_preProc;
    if (is_task_proc(p_proc)) {        /* 系统任务？ */
        /* 如果系统任务的堆栈已经不完整，内核出错。 */
        if (*p_proc->stackGuardWord != SYS_TASK_STACK_GUARD) {
            panic("stack over run by task", p_proc->pid);
        }

        p_preProc = gp_readyHeads[TASK_QUEUE];   /* 得到就绪队列头的进程 */
        if (p_preProc == NIL_PROC) return;     /* 并无就绪的系统任务 */
        if (p_preProc == p_proc) {
            /* 如果就绪队列头的进程就是我们要让之堵塞的进程，那么我们将它移除出就绪队列 */
            gp_readyHeads[TASK_QUEUE] = p_preProc->p_nextReady;
//            printf("p_preProc_%d ",p_preProc->logicNum);
//            printf("h_%d",gp_readyHeads[TASK_QUEUE]->logicNum);
            if (p_proc == gp_curProc) {
                /* 如果堵塞的进程就是当前正在运行的进程，那么我们需要重新狩猎以得到一个新的运行进程 */
                hunter();
            }
            return;
        }
        /* 如果这个进程不在就绪队列头，那么搜索整个就绪队列寻找它 */
        while (p_preProc->p_nextReady != p_proc) {
            p_preProc = p_preProc->p_nextReady;
            if (p_preProc == NIL_PROC) return;   /* 到边界了，说明这个进程根本就没在就绪队列内 */
        }
        /* 找到了，一样，从就绪队列中移除它 */
        p_preProc->p_nextReady = p_preProc->p_nextReady->p_nextReady;
        /* 如果之前队尾就是要堵塞的进程，那么现在我们需要重新调整就绪队尾指针（因为它现在指向了一个未就绪的进程） */
        if (gp_readyTails[TASK_QUEUE] == p_proc) gp_readyTails[TASK_QUEUE] = p_preProc;   /* 现在找到的p_preProc进程是队尾 */
    } else if (is_serv_proc(p_proc)) {     /* 系统服务 */
        /* 所作操作同上的系统任务一样 */
        p_preProc = gp_readyHeads[SERVER_QUEUE];
        if (p_preProc == NIL_PROC) return;
        if (p_preProc == p_proc) {
            gp_readyHeads[SERVER_QUEUE] = p_preProc->p_nextReady;
            /* 这里注意，因为不是系统任务，我们不作那么严格的判断了 */
            hunter();
            return;
        }
        while (p_preProc->p_nextReady != p_proc) {
            p_preProc = p_preProc->p_nextReady;
            if (p_preProc == NIL_PROC) return;
        }
        p_preProc->p_nextReady = p_preProc->p_nextReady->p_nextReady;
        if (gp_readyTails[SERVER_QUEUE] == p_proc) gp_readyTails[SERVER_QUEUE] = p_preProc;
    } else {                           /* 用户进程 */
        p_preProc = gp_readyHeads[USER_QUEUE];
        if (p_preProc == NIL_PROC) return;
        if (p_preProc == p_proc) {
            gp_readyHeads[USER_QUEUE] = p_preProc->p_nextReady;
            /* 这里注意，因为不是系统任务，我们不作那么严格的判断了 */
            hunter();
            return;
        }
        while (p_preProc->p_nextReady != p_proc) {
            p_preProc = p_preProc->p_nextReady;
            if (p_preProc == NIL_PROC) return;
        }
        p_preProc->p_nextReady = p_preProc->p_nextReady->p_nextReady;
        if (gp_readyTails[USER_QUEUE] == p_proc) gp_readyTails[USER_QUEUE] = p_preProc;
    }
}

/* 停止进程调度 */
PUBLIC void schedule_stop(void) {
    /**
     * 本例程只针对用户进程，使其用户进程不能再被调度,通常在系统宕机时
     * 本例程会被调用，因为这时候系统及其不可靠。本例程的实现也非常简单
     * ，让用户就绪队列为空即可，这样调度程序就找不到任何用户进程了。
     */
    gp_readyHeads[USER_QUEUE] = NIL_PROC;
}

PUBLIC void interrupt(int task) {

    /**
     * 在接受到一条硬件中断后，相应设备的底层中断服务例程将调用该函数。
     * 功能是将中断转换为向该设备所对应的系统任务发送一条消息，而且通常
     * 在调用 interrupt 之前几乎不进行什么操作。
     */
    register Process *p_target = proc_addr(task);   /* 要中断的任务 */

    /**
     * 如果发生中断重入或正在发送一个进程切换，则将当前中断加入排队队列，函数到此结束，
     * 当前挂起的中断将在以后调用 unhold 时再处理。
     */
//    if(kernelReenter != 0 || switching) {
//        interrupt_lock();
//        /**
//         * 如果进程没有中断被挂起正在等待处理时才继续
//         * 这样做是为了保证一个任务的中断不会重复的被挂起，因为这是无用功，
//         * 最重要的是让任务尽快完成首次被挂起的中断
//         */
//        if(!p_target->intHeld) {
//            p_target->intHeld = TRUE;
//            if(gp_heldHead == NIL_PROC)
//                gp_heldHead = p_target;
//            else
//                gp_heldTail->p_nextHeld = p_target;
//            gp_heldTail = p_target;             /* 无论如何，尾指针都指向最新挂起的 p_target */
//            p_target->p_nextHeld = NIL_PROC;
//        }
//        interrupt_unlock();
//        return;
//    }


    /**
     * 现在检查任务是否正在等待一个中断，如果任务未做好接收中断准备，则其 int_blocked
     * 标志被置位 - 在 ipc.c 文件中的 aos_receive 接收消息例程中我们将使得丢失的
     * 中断可能被恢复，并且不需要发送消息(@TODO)。
     */
    if ((p_target->flags & (RECEIVING | SENDING)) != RECEIVING || /* 不处于单纯的接收消息的状态 */
        !is_any_hardware(p_target->getFrom)) {
        if (intMsgsSize == intMsgsCapacity) {   /* 队列已满 */
            p_target->intBlocked = TRUE;    /* 中断被堵塞 */
            kprintf("ignore int to %d, attention!!!\n", task);
            return;
        }
        int i = 0;
        while (intMsgs[i].to == 0) i++; /* todo 更改0 */
        intMsgs[i].to = p_target->pid;
        intMsgs[i].msg.source = HARDWARE;
        intMsgs[i].msg.type = HARD_INT;
        intMsgsSize++;
        kprintf("int msg into queue.\n");
        return;
    }

    /**
     * 通过上面的测试，现在被中断的系统任务可以接收一条硬件消息了，我们我们开始向其发送消息。
     * 从 HARDWARE(代表计算机硬件)向系统任务发送消息是很简单的，因为任务和核心是编译到同一个文件
     * 中的，因此可以访问相同的数据区域，直接赋值即可。
     */
    p_target->inBox->source = HARDWARE;
    p_target->inBox->type = HARD_INT;
    p_target->flags &= ~RECEIVING;
    p_target->intBlocked = FALSE;

    /* 进程就绪例程 ready 例程的在线代码替换
     * 因为从中断产生的消息只会发送到系统任务，这样便无需确定操作的进程队列了。
     */
    if (gp_readyHeads[TASK_QUEUE] != NIL_PROC)
        gp_readyTails[TASK_QUEUE]->p_nextReady = p_target;
    else
        gp_curProc = gp_readyHeads[TASK_QUEUE] = p_target;
    gp_readyTails[TASK_QUEUE] = p_target;
    p_target->p_nextReady = NIL_PROC;
}

/* 处理挂起的中断 */
PUBLIC void unhold(void) {
    /**
     * 遍历被挂起的中断队列，使用 interrupt 函数去处理每个中断，
     * 其目的是在另一个进程被允许运行之前将每一条挂起的中断转换
     * 成一个消息处理掉。
     */
    register Process *p_target; /* 指向挂起的中断队列成员 */

    /* 如果进程正在切换，或队列为空，下次一定 */
    if (switching || gp_heldHead == NIL_PROC) return;

    p_target = gp_heldHead;
    do {
        gp_heldHead = gp_heldHead->p_nextHeld;   /* 队列下一个成员 */
        interrupt_lock();
        interrupt(p_target->pid);        /* 产生一条硬件消息给它 */
        interrupt_unlock();
    } while ((p_target = gp_heldHead) != NIL_PROC);
    gp_heldTail = NIL_PROC;                   /* 已经处理完毕，尾指针也指向 NULL */
}


/* 加锁的hunter */
PUBLIC void lock_hunter(void) {
    switching = TRUE;
    hunter();
    switching = FALSE;
}


PUBLIC void lock_ready(Process *proc) {
//    kprintf("a new proc, name:%s  \n", proc->name);
    switching = TRUE;
    ready(proc);
    switching = FALSE;
}

PUBLIC void lock_unready(Process *proc) {
    switching = TRUE;
    unready(proc);
    switching = FALSE;
}

PUBLIC void lock_schedule(void) {
    switching = TRUE;
    schedule();
    switching = FALSE;
}

/* 狩猎一个进程用于下次执行 */
PRIVATE void hunter(void) {

    /* 从进程表中抓出一个作为下次运行的进程 */
    register Process *prey;      /* 准备运行的进程 */
    if ((prey = gp_readyHeads[TASK_QUEUE]) != NIL_PROC) {
        gp_curProc = prey;
//        kprintf("%s hunter\n", gp_curProc->name);
        return;
    }
    if ((prey = gp_readyHeads[SERVER_QUEUE]) != NIL_PROC) {
        gp_curProc = prey;
        return;
    }
    if ((prey = gp_readyHeads[USER_QUEUE]) != NIL_PROC) {
        gp_billProc = gp_curProc = prey;
//        kprintf("%s hunter\n", gp_curProc->name);
        return;
    }

    /* 咳咳，本次狩猎失败了，那么只能狩猎 IDLE 待机进程了 */
    prey = proc_addr(IDLE_TASK);
    gp_billProc = gp_curProc = prey;
    /* 本例程只负责狩猎，狩猎到一个可以执行的进程，而进程执行完毕后的删除或更改在队列中的位置
     * 这种事情我们安排在其他地方去做。
     */
//    if(gp_curProc->logicNum==IDLE_TASK){
//        printf("idle hunt:%d\n",prey->logicNum);
//    }
}

/* 进程调度 */
PRIVATE void schedule(void) {
    /**
     * 这个调度程序只针对于用户进程
     * 尽管多数调度决策实在一个进程阻塞或解除阻塞时作出的，但调度仍要考虑
     * 到当前用户进程时间片用完的情况。这种情况下，时钟任务调度 schedule 来将
     * 就绪用户进程队首的进程移到队尾。
     * 该算法的结果是将用户进程按时间片轮转方式运行。文件系统、内存管理器
     * 和I/O任务绝不会被放在队尾，因为它们肯定不会运行得太久。这些进程可以
     * 被认为是非常可靠的，因为它们是我们编写的，而且在完成要做的工作后将堵塞。
     */

    /* 如果没有准备好的用户进程，请返回 */
    if (gp_readyHeads[USER_QUEUE] == NIL_PROC) return;

    /* 将队首的进程移到队尾 */
    Process *p_tmp;
    p_tmp = gp_readyHeads[USER_QUEUE]->p_nextReady;
    gp_readyTails[USER_QUEUE]->p_nextReady = gp_readyHeads[USER_QUEUE];
    gp_readyTails[USER_QUEUE] = gp_readyTails[USER_QUEUE]->p_nextReady;
    gp_readyHeads[USER_QUEUE] = p_tmp;
    gp_readyTails[USER_QUEUE]->p_nextReady = NIL_PROC;  /* 队尾没有后继进程 */
    /* 汉特儿 */
    hunter();
}

/**
 * 通过进程pid得到进程的逻辑索引
 * @param pid 进程pid
 * @return 进程的logicIndex或NO_TASK
 */
PUBLIC int get_logicI(pid_t pid) {
    if (pid <= 0) return pid;
    Process *p_proc;
    for (int i = 1; i < NR_PROCS; i++) {
        p_proc = proc_addr(i);
        if (p_proc->pid == pid) return p_proc->logicIndex;
    }
    return NO_TASK;
}