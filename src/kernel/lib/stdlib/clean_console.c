//
// Created by 杜科 on 2021/1/24.
//

#include "core/kernel.h"

PUBLIC int clean_console() {
    Message msg;
    msg.type = CLEAR;
    send_rec(TTY_TASK, &msg);

    return 0;
}