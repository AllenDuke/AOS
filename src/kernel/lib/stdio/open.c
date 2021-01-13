//
// Created by 杜科 on 2021/1/6.
//
#include "core/kernel.h"
#include "../include/stdio.h"

/*****************************************************************************
 *                                open
 *****************************************************************************/
/**
 * open/create a file.
 *
 * @param pathname  The full path of the file to be opened/created.
 * @param flags     O_CREAT, O_RDWR, etc.
 *
 * @return File descriptor if successful, otherwise -1.
 *****************************************************************************/
PUBLIC int open(const char *pathname, int flags)
{
    Message msg;

    msg.type	= OPEN;

    msg.PATHNAME	= (void*)pathname;
    msg.FLAGS	= flags;
    msg.NAME_LEN	= strlen(pathname);

    send_rec(FS_TASK, &msg);
    assert(msg.type == SYSCALL_RET);
//    if(msg.type != SYSCALL_RET) panic("msg type err\n",msg.type);
    return msg.FD;
}
