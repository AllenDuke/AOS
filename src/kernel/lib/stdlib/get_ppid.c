//
// Created by 杜科 on 2021/1/26.
//

#include "core/kernel.h"

PUBLIC int get_ppid(){

    Message msg;
    msg.type = GET_PPID;
    send_rec(MM_TASK, &msg);

    return msg.PID;
}