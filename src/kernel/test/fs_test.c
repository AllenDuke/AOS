//
// Created by 杜科 on 2021/1/6.
//
#include "core/kernel.h"
#include "stdio.h"

PUBLIC void fs_test(){
    int fd;
    int n;
    const char filename[] = "test";
    const char bufw[] = "abcde";
    const int rd_bytes = 3;
    char bufr[rd_bytes];

/* 第一次调用时创建，已存在时调用会出错 */
////    assert(rd_bytes <= strlen(bufw));
//    if(rd_bytes > strlen(bufw)) panic("rd_bytes is too long\n",rd_bytes);
//
//    /* create */
//    fd = open(filename, O_CREAT | O_RDWR);
////    assert(fd != -1);
//    if(fd==-1) panic("fd err\n",PANIC_ERR_NUM);
//    kprintf("File created. fd: %d\n", fd);
//
//    /* write */
//    n = write(fd, bufw, strlen(bufw));
////    assert(n == strlen(bufw));
//    if(n!=strlen(bufw)) panic("write err\n",n);
//
//    /* close */
//    close(fd);

/* 已存在时调用 */
    /* open */
    fd = open(filename, O_RDWR);
    assert(fd != -1);
//    if(fd==-1) panic("fd err\n",PANIC_ERR_NUM);
    kprintf("File opened. fd: %d\n", fd);

    /* read */
    n = read(fd, bufr, rd_bytes);
    assert(n == rd_bytes);
//    if(n!=rd_bytes) panic("write err\n",n);
    bufr[n] = 0;
    kprintf("%d bytes read: %s\n", n, bufr);

    /* close */
    close(fd);

    park();
}