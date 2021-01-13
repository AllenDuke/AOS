//
// Created by 杜科 on 2021/1/12.
//

#include "core/kernel.h"
#include "../include/stdio.h"

/**
 * 等待一个子进程结束
 * @param waitPid -1为等待所有的子进程
 * @return
 */
PUBLIC int wait(int pid) {
    Message msg;
    msg.type = WAIT;

    send_rec(MM_TASK, &msg);
    msg.PID=pid;

    return (msg.PID == NO_TASK ? -1 : msg.PID);
}