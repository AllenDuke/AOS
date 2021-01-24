//
// Created by 杜科 on 2021/1/24.
//
#include "stdio.h"
#include "stdlib.h"
#include "../origin.h"
#include "errno.h"

int touch(int argc, char *argv[]) {
    if(argc!=2) {
        printf("arg err!\n");
        return -1;
    }

    int fd = open(argv[1], O_CREAT | O_RDWR);
    if(fd==-1){
        printf("file creat err!\n");
        return ENOENT;
    }

    printf("File created.\n");
    return 0;
}
