//
// Created by 杜科 on 2020/12/28.
//

#include "core/kernel.h"

PRIVATE Message msg;

PRIVATE void checkFAT32();

PUBLIC void fs_task(void){
    in_outbox(&msg,&msg);
    kprintf("fs_task working...\n");
    while (TRUE){
        rec(ANY);
    }
}

PRIVATE void checkFAT32(){

}