//
// Created by 杜科 on 2021/1/10.
//
#include "core/kernel.h"
#include "../include/stdio.h"

//extern void panic(const char *p_msg, int errorNum);
PUBLIC void exit(int status) {
    Message msg;
    msg.type = EXIT;
    msg.STATUS = status;

    send_rec(MM_TASK, &msg);
//    assert(msg.type == SYSCALL_RET);
    if (msg.type != SYSCALL_RET) panic("exit get wrong msg type\n", msg.type);
}