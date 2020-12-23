//
// Created by 杜科 on 2020/12/22.
//
#include "core/kernel.h"

PRIVATE Message message;

PRIVATE u8_t parse(char* cmd);

PUBLIC void init_task(void) {

    kprintf("#{Init}-> Working...\n");
    in_outbox(&message,&message);

    while (TRUE) {
        receive(TTY_TASK,&message);
        kprintf("get msg:%s\n",message.source);

    }

}

PRIVATE u8_t parse(char* cmd){

}