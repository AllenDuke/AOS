//
// Created by 杜科 on 2021/1/12.
//

#include "core/kernel.h"
#include "stdio.h"

void origin() {
    int fd_stdin = open("/dev_tty0", O_RDWR);
//    assert(fd_stdin == 0);
    int fd_stdout = open("/dev_tty0", O_RDWR);
//    assert(fd_stdout == 1);

    printf("origin() is running ...\n");


    int pid = fork();
    if (pid != 0) { /* parent process */
        int s;
        int child = wait(&s);
        printf("child (%d) exited with status: %d.\n", child, s);
    } else {    /* child process */
        execl("/echo", "echo", "hello", "world", 0);
    }

    while (1) {
        int s;
        int child = wait(&s);
        printf("child (%d) exited with status: %d.\n", child, s);
    }

}