//
// Created by 杜科 on 2021/1/24.
//
#include "stdio.h"
#include "stdlib.h"
#include "../origin.h"
#include "errno.h"

/* 不同于linux，要求文件必须存在，只能在末尾追加 */
int vi(int argc, char *argv[]) {
    if (argc != 2) {
        printf("arg err!\n");
        return -1;
    }

    int fd_in = open(argv[1], O_RDWR);
    if (fd_in == -1) {
        printf("no such file.\n");
        return ENOENT;
    }
    int fd_out = open(argv[1], O_RDWR);
    if (fd_out == -1) {
        printf("no such file.\n");
        return ENOENT;
    }

    int fd_stdin = open("/dev_tty1", O_RDWR);
    int fd_stdout = open("/dev_tty1", O_RDWR);
    chang_console(1);       /* 这里仍要切换，因为tty任务处理的当前屏幕 */

    clean_console();           /* 清屏 */

    int size = 1024;        /* 暂时限制文件大小为1024字节 */
    char buf[size];

    read(fd_in, buf, size);

    int n = 0;                /* 不可以直接使用read的返回值，因为它以512字节为单位 */
    while (buf[n] != 0) n++;
//    pprintf(fd_stdout,"read:%d.\n",n);

    pprintf(fd_stdout, "%s", buf);

    int c = read(fd_stdin, buf + n, size);      /* 读取键盘输入 */
    buf[n + c] = 0;         /* 写入一个结尾 */

    n = write(fd_out, buf, n + c + 1);              /* 写到文件 */

    close(fd_in);
    close(fd_out);

    close(fd_stdin);
    close(fd_stdout);

    chang_console(0);

    printf("writen:%d.\n", c);
    return 0;
}