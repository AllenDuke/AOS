//
// Created by 杜科 on 2021/1/12.
//

#include "core/kernel.h"
#include "../include/stdio.h"

PUBLIC int wait(int *status) {
    Message msg;
    msg.type = WAIT;

    send_rec(MM_TASK, &msg);

    *status = msg.STATUS;

    return (msg.PID == NO_TASK ? -1 : msg.PID);
}