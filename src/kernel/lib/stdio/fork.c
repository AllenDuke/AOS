//
// Created by 杜科 on 2021/1/13.
//

#include "core/kernel.h"
#include "../include/stdio.h"

/**
 * fork出一条子进程（父进程的一份复制）。
 * @return
 * 成功时，父进程得到子进程的pid，子进程得到0。
 * 失败时，父进程得到-1，子进程创建失败。
 */
PUBLIC int fork() {
    Message msg;
    msg.type = FORK;
    msg.LEVEL = 1;    /* 默认等级为1 */

    send_rec(MM_TASK, &msg);
//    assert(msg.type == SYSCALL_RET); /* assert位于ring0，不能使用 */
//    assert(msg.RETVAL == 0);

    return msg.PID;
}

PUBLIC int fork_level(u8_t level) {
    if (level < 1 || level > 5) return -1;

    Message msg;
    msg.type = FORK;
    msg.LEVEL = level;

    send_rec(MM_TASK, &msg);
//    assert(msg.type == SYSCALL_RET); /* assert位于ring0，不能使用 */
//    assert(msg.RETVAL == 0);

    return msg.PID;
}