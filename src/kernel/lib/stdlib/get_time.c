//
// Created by 杜科 on 2021/1/18.
//

#include "core/kernel.h"

PUBLIC long get_time(){
    Message msg;
    msg.type = GET_TIME;
    send_rec(CLOCK_TASK, &msg);

    return msg.CLOCK_TIME;
}