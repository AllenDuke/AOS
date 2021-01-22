//
// Created by 杜科 on 2021/1/12.
//

#include "core/kernel.h"
#include "../include/stdio.h"

/**
 * 等待一个子进程结束（需要两者为父子关系）
 * @param waitPid -1为等待所有的子进程
 * @return 成功返回子进程pid，错误返回-1
 */
PUBLIC int waitpid(pid_t pid) {
    if (pid < 0 && pid != -1) return -1;       /* 如果是负数但不是-1，那么直接返回-1，不进行系统调用 */

    Message msg;
    msg.type = WAIT;
    msg.PID = pid;

    send_rec(MM_TASK, &msg);

    return (msg.PID == NO_TASK ? -1 : msg.PID);
}

PUBLIC int wait() {
    Message msg;
    msg.type = WAIT;
    msg.PID = -1;

    send_rec(MM_TASK, &msg);

    return (msg.PID == NO_TASK ? -1 : 1);
}

/**
 * 等待一个子进程结束（需要两者为父子关系），接收子进程退出时的状态
 * @param pid 非负数 特定的子进程
 * @param status 退出状态指针
 * @return 成功返回子进程pid，错误返回-1
 */
PUBLIC int waitpid_stat(pid_t pid, u8_t *status) {
    if (pid < 0) return -1;       /* 如果是负数，那么直接返回-1，不进行系统调用 */

    Message msg;
    msg.type = WAIT;
    msg.PID = pid;
    msg.STATUS = STATUS_NEED;

    send_rec(MM_TASK, &msg);

    *status = msg.STATUS;

    return (msg.PID == NO_TASK ? -1 : msg.PID);
}