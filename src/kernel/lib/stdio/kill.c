//
// Created by 杜科 on 2021/2/3.
//

#include "core/kernel.h"
#include "../include/stdio.h"


PUBLIC int kill_proc(int pid) {
    if (pid <= 0) return -1;
    Message msg;
    msg.type = KILL;
    msg.PID = pid;

    send_rec(MM_TASK, &msg);

    return 1;
//    assert(msg.type == SYSCALL_RET);
}