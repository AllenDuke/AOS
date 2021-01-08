//
// Created by 杜科 on 2021/1/7.
//

#include "core/kernel.h"

void tty_test()
{
    char tty_name[] = "/dev_tty0";

    int fd_stdin  = open(tty_name, O_RDWR);
//    assert(fd_stdin  == 0);
    if(fd_stdin  != 0) panic("tty0 fd err\n",fd_stdin);
    int fd_stdout = open(tty_name, O_RDWR);
//    assert(fd_stdout == 1);
    if(fd_stdout  != 1) panic("tty0 fd err\n",fd_stdin);

    char rdbuf[128];

    while (1) {
        printf("$ ");
        int r = read(fd_stdin, rdbuf, 70);
        rdbuf[r] = 0;

        if (strcmp(rdbuf, "hello") == 0)
            printf("hello world!\n");
        else
        if (rdbuf[0])
            printf("{%s}\n", rdbuf);
    }

}