//
// Created by 杜科 on 2021/1/9.
//

#include "core/kernel.h"
#include "../include/stdio.h"

/*****************************************************************************
 *                                stat
 *************************************************************************//**
 *
 *
 * @param path
 * @param buf
 *
 * @return  On success, zero is returned. On error, -1 is returned.
 *****************************************************************************/
PUBLIC int stat(const char *path, struct stat *buf)
{
    Message msg;

    msg.type	= STAT;

    msg.PATHNAME	= (void*)path;
    msg.BUF		= (void*)buf;
    msg.NAME_LEN	= strlen(path);

    send_rec(FS_TASK, &msg);
    assert(msg.type == SYSCALL_RET);
//    if(msg.type!=SYSCALL_RET) panic("msg type err\n",msg.type);

    return msg.RETVAL;
}