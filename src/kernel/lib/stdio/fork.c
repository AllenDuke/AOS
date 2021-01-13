//
// Created by 杜科 on 2021/1/13.
//

#include "core/kernel.h"
#include "../include/stdio.h"

/*****************************************************************************
 *                                fork
 *****************************************************************************/
/**
 * Create a child process, which is actually a copy of the caller.
 *
 * @return   On success, the PID of the child process is returned in the
 *         parent's thread of execution, and a 0 is returned in the child's
 *         thread of execution.
 *           On failure, a -1 will be returned in the parent's context, no
 *         child process will be created.
 *****************************************************************************/
PUBLIC int fork()
{
    Message msg;
    msg.type = FORK;

    send_rec(MM_TASK, &msg);
    assert(msg.type == SYSCALL_RET);
    assert(msg.RETVAL == 0);

    return msg.PID;
}