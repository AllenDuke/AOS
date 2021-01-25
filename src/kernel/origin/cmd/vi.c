//
// Created by 杜科 on 2021/1/24.
//
#include "stdio.h"
#include "stdlib.h"
#include "../origin.h"
#include "errno.h"

/* 不同于linux，要求文件必须存在 */
int vi(int argc, char *argv[]) {
    if (argc != 2) {
        printf("arg err!\n");
        return -1;
    }

    int fd_in = open(argv[1], O_RDWR);
    if(fd_in==-1){
        printf("no such a file.\n");
        return ENOENT;
    }
    int fd_out = open(argv[1], O_RDWR);
    if(fd_out==-1){
        printf("no such a file.\n");
        return ENOENT;
    }

    int fd_stdin = open("/dev_tty1", O_RDWR);
    int fd_stdout = open("/dev_tty1", O_RDWR);
    chang_console(1);

    clean_console();

    int size = 512;
    char buf[size];

    int n = read(fd_in, buf, size);
    buf[n]=0;
    pprintf(fd_stdout,"read:%d.\n",n);

    pprintf(fd_stdout,"%s",buf);

    n=read(fd_stdin,buf,size);

    n=write(fd_out,buf,n);

    pprintf(fd_stdout,"writen:%d.\n",n);

    close(fd_in);
    close(fd_out);

    close(fd_stdin);
    close(fd_stdout);

    chang_console(0);
    return 0;
}