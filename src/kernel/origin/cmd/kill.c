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
    int pid = 0;
    char *nums = argv[1];
//    printf("pid :%s\n",nums);
    while (*nums != 0) {
        pid = pid * 10 + (*nums - '0');
        nums++;
    }
    if (kill_proc(pid) == -1) {
        printf("no such proc:%d !\n",pid);
    };
    return 0;
}