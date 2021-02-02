//
// Created by 杜科 on 2021/2/2.
//

#include "stdlib.h"
#include "../origin.h"
#include "stdio.h"

/* 与linux不同，这里清理当前console所属的整段显存 */
int check_proc(int argc, char *argv[]) {
    int fd_stdin = open("/dev_tty2", O_RDWR);
    chang_console(2);
    char buf[3];
    read(fd_stdin, buf, 3);     /* 读取键盘输入 */
    clean_console();                /* 清除换行 */
    close(fd_stdin);
    chang_console(0);
    return 0;
}