//
// Created by 杜科 on 2021/1/10.
//
#include "core/kernel.h"
#include "../include/stdio.h"

/**
 * 进程退出时调用
 * @param status 这个值来自于main函数的返回值。
 */
PUBLIC void exit(int status) {
    Message msg;
    msg.type = EXIT;
    msg.STATUS = status;

    send_rec(MM_TASK, &msg);
//    assert(msg.type == SYSCALL_RET);
}