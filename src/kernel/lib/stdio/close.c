//
// Created by 杜科 on 2021/1/6.
//
#include "core/kernel.h"

/*****************************************************************************
 *                                close
 *****************************************************************************/
/**
 * Close a file descriptor.
 *
 * @param fd  File descriptor.
 *
 * @return Zero if successful, otherwise -1.
 *****************************************************************************/
PUBLIC int close(int fd)
{
    Message msg;
    msg.type   = CLOSE;
    msg.FD     = fd;

    send_rec(FS_TASK, &msg);

    return msg.RETVAL;
}