//
// Created by 杜科 on 2021/1/24.
//

#include "core/kernel.h"

PUBLIC int change_console(int n) {
    if (n < 0 || n > NR_CONSOLES) return -1;

    Message msg;
    msg.type = CONSOLE_CHANGE;
    msg.CONSOLE = n;
    send_rec(TTY_TASK, &msg);

    return 0;
}