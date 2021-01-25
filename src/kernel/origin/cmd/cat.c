//
// Created by 杜科 on 2021/1/24.
//
#include "stdio.h"
#include "stdlib.h"
#include "../origin.h"
#include "errno.h"

int cat(int argc, char *argv[]) {
    if(argc!=2) {
        printf("arg err!\n");
        return -1;
    }

    int fd = open(argv[1], O_RDWR);
    if(fd==-1){
        printf("no such file.\n");
        return ENOENT;
    }

    int size = 512;
    char buf[size];

    /* read */
    int n = read(fd, buf, size);
    buf[n]=0;

    printf("%s\n",buf);
    return 0;
}