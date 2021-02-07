//
// Created by 杜科 on 2021/2/3.
//

#include "stdio.h"
#include "../origin.h"

int kill(int argc, char *argv[]) {
    if (argc != 2) {
        printf("arg err!\n");
        return -1;
    }
    int pid = *argv[1] - '0';
    if(kill_proc(pid)==-1){
        printf("no such proc!\n");
    };
    return 0;
}