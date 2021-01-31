//
// Created by 杜科 on 2021/1/30.
//

#include "core/kernel.h"
#include "stdio.h"

PUBLIC int get_alive_proc(char *pids){

    Message msg;
    msg.type = GET_ALIVE_PROC;
    msg.PIDS=pids;
    send_rec(MM_TASK, &msg);

    return 1;
}