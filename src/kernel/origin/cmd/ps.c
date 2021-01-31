//
// Created by 杜科 on 2021/1/30.
//

#include "stdio.h"
#include "stdlib.h"
#include "../origin.h"

/* 打印当前存活的用户进程 */
int ps(int argc, char *argv[]) {
    printf("PID\n");
    char ids[32];
    char *pids = ids;
//    printf("addr:%d\n", pids);
    get_alive_proc(pids);
    for (int i = 0; i < 32; ++i) {
        if (*pids == -1) continue;
        printf("%d\n", *pids);
        pids++;
    }
    return 0;
}
