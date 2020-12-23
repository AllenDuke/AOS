//
// Created by 杜科 on 2020/12/22.
//
#include "core/kernel.h"

PRIVATE Message msg;

PUBLIC void init_task(void) {

    in_outbox(&msg,&msg);

    while (TRUE) {
        kprintf("aos:/$ ");
        receive(TTY_TASK,&msg);
        kprintf("aos:/$ ");
//        kprintf("get cmd: %d\n",msg.type);
        switch (msg.type) {
            case 0:{
                kprintf("default cmd");
                break;
            }
            case 1:{
                kprintf("default cmd");
                break;
            }
            case 2:{
                kprintf("/");
                break;
            }
            case 3:{
                msg.source=INIT_TASK;
                msg.type=GET_TIME;
                send_rec(CLOCK_TASK,&msg);
                kprintf("current date is: %d",msg.CLOCK_TIME);
            }
            case 4:{
                kprintf("default cmd");
                break;
            }
            case 5:{
                kprintf("default cmd");
                break;
            }
            case 6:{
                kprintf("default cmd");
                break;
            }
            case 7:{
                kprintf("default cmd");
                break;
            }
        }
    }

}