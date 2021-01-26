//
// Created by 杜科 on 2021/1/26.
//

#include "stdio.h"
#include "stdlib.h"
#include "../origin.h"
#include "errno.h"

/* 展示进程切换 */
int show_proc(int argc, char *argv[]) {

//    int count = 4;
//    for (int i = 0; i < count; ++i) {
//        int pid = fork();
//        if (pid == 0) {
//            printf("pid:%d.\n",get_pid());
//            exit(0);
//        } else
//            printf("son:%d.\n", pid);
//    }

    int pid = fork();
    if (pid == 0) {
        pid=get_pid();
        printf("pid:%d.\n", pid);
        exit(1);
    }
    unsigned char exitStat;
    pid=waitpid_stat(pid,&exitStat);
    printf("son exit:%d.\n",exitStat);

    return pid;
}