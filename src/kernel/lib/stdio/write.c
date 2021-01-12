//
// Created by 杜科 on 2021/1/6.
//

#include "core/kernel.h"
#include "../include/stdio.h"

/*****************************************************************************
 *                                write
 *****************************************************************************/
/**
 * Write to a file descriptor.
 *
 * @param fd     File descriptor.
 * @param buf    Buffer including the bytes to write.
 * @param count  How many bytes to write.
 *
 * @return  On success, the number of bytes written are returned.
 *          On error, -1 is returned.
 *****************************************************************************/
PUBLIC int write(int fd, const void *buf, int count)
{
    Message msg;
    msg.type = WRITE;
    msg.FD   = fd;
    msg.BUF  = (void*)buf;
    msg.COUNT  = count;

    send_rec(FS_TASK, &msg);

    return msg.COUNT;
}

