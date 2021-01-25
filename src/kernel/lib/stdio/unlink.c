//
// Created by 杜科 on 2021/1/12.
//

#include "core/kernel.h"
#include "../include/stdio.h"

PUBLIC int unlink(const char *pathname) {
    Message msg;
    msg.type = UNLINK;

    msg.PATHNAME = (void *) pathname;
    msg.NAME_LEN = strlen(pathname);

    send_rec(FS_TASK, &msg);

    return msg.RETVAL;
}