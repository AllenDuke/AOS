//
// Created by 杜科 on 2021/1/31.
//

#include "core/kernel.h"
#include "stdio.h"

PUBLIC int dump_pm(){

    Message msg;
    msg.type = TOP;
    send_rec(MM_TASK, &msg);

    return 1;
}