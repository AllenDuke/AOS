//
// Created by 杜科 on 2021/1/31.
//

#include "stdio.h"
#include "stdlib.h"
#include "../origin.h"

/* 打印当前存活的用户进程 */
int top(int argc, char *argv[]) {
    dump_pm();
    return 0;
}