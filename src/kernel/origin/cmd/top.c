//
// Created by 杜科 on 2021/1/31.
//

#include "stdio.h"
#include "stdlib.h"
#include "../origin.h"

/* 打印当前存活的用户进程 */
int top(int argc, char *argv[]) {
    int fd_stdin = open("/dev_tty2", O_RDWR);
    change_console(2);
    char buf[3];
    read(fd_stdin, buf, 3);     /* 读取键盘输入 */
    clean_console();                /* 清除换行 */
    close(fd_stdin);
    change_console(0);
    return 0;
}