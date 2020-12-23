//
// Created by 杜科 on 2020/12/14.
//

#include "core/kernel.h"

/* 空闲待机任务 */
PUBLIC void idle_task(void) {
    /**
     * 本例程是一个空循环，AOS 系统没有任何进程就绪时，则会调用本例程，idle不会进入就绪队列。
     * 本例程的循环体使用了 hlt 指令，使其 CPU 暂停工作并处于待机等待状态，
     * 不至于像传统的死循环一样，消耗大量的 CPU 资源。而且在每个待机的过程
     * 中都会保持中断开启，保证待机时间内随时可以响应活动。
     */
//    kprintf("idle task work\n");
    while (TRUE){
        level0(halt);
    }
}