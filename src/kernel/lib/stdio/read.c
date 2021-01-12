//
// Created by 杜科 on 2021/1/6.
//

#include "core/kernel.h"
#include "../include/stdio.h"

/*****************************************************************************
 *                                read
 *****************************************************************************/
/**
 * Read from a file descriptor.
 *
 * @param fd     File descriptor.
 * @param buf    Buffer to accept the bytes read.
 * @param count  How many bytes to read.
 *
 * @return  On success, the number of bytes read are returned.
 *          On error, -1 is returned.
 *****************************************************************************/
PUBLIC int read(int fd, void *buf, int count)
{
    Message msg;
    msg.type = READ;
    msg.FD   = fd;
    msg.BUF  = buf;
    msg.COUNT  = count;

    send_rec(FS_TASK, &msg);

    return msg.COUNT;
}