//
// Created by 杜科 on 2021/1/25.
//
#include "stdio.h"
#include "stdlib.h"
#include "../origin.h"
#include "errno.h"

/* 删除一个文件，要求文件必须存在 */
int rm(int argc, char *argv[]) {
    if(argc!=2) {
        printf("arg err!\n");
        return -1;
    }

    /* 先将文件的第一个字符置为0，避免下一次touch的时候文件内容复现。因为unlink不会改变文件的内容 */
    int fd_out = open(argv[1], O_RDWR);
    if (fd_out == -1) {
        printf("no such file.\n");
        return ENOENT;
    }
    char buf[1];
    buf[0]=0;
    write(fd_out, buf,1);
    close(fd_out);

    int re=unlink(argv[1]);
    printf("file removed.\n");
    return re;
}