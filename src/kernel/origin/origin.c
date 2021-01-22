//
// Created by 杜科 on 2021/1/12.
//

/**
 * 这里的origin进程其实是和内核编译链接在一起的，整个内核的大小就是origin的大小。
 * 实际上它可以变得更小，比如独立编译，然后将其写进硬盘，最后内核将其读到内存里运行。
 *
 * origin的数据尽量在栈上，否则fork后的子进程，访问的数据可能会有异常。所以origin的栈尽可能大一点。
 */
#include "origin.h"
#include "stdio.h"

PRIVATE void exec_cmd(char *cmdBuf, int cmdLen, HashTableNode hashTable[]);

PRIVATE void start(HashTableNode *node);

void origin_task() {

    int fd_stdin = open("/dev_tty0", O_RDWR);
    assert(fd_stdin == 0);
    int fd_stdout = open("/dev_tty0", O_RDWR);
    assert(fd_stdout == 1);

    HashTableNode hashTable[NR_CMD_TSIZE];      /* HashTable，冲突时线性探测，2倍是有想法的 */

    put("pwd", 3, pwd, hashTable);
    put("date", 4, date, hashTable);

    printf("{ORIGIN}->origin_task is working...\n");

    char cmdBuf[128];
    int cmdLen = 0;

    printf("$ ");
    while (1) {
        int r = read(fd_stdin, cmdBuf, 70);
        cmdLen = r;
        cmdBuf[r] = 0;

        int pid = fork();
        if (pid == 0) {
            exec_cmd(cmdBuf, cmdLen, hashTable);
        } else {
            unsigned char exitStat;
            printf("child pid:%d.\n", pid);
            waitpid_stat(pid, &exitStat);
            printf("child (%d) exited with status: %d.\n", pid, exitStat);
            printf("$ ");
        }
    }

}

/**
 * 一个伪装的exec命令。
 * 实际上，它应该完成这样的事情，利用文件系统，把命令对应的文件读进内存，然后设置好该进程相关信息。
 */
PRIVATE void exec_cmd(char *cmdBuf, int cmdLen, HashTableNode hashTable[]) {
    HashTableNode *node = get(cmdBuf, cmdLen, hashTable);
    start(node);
}

/**
 * 一个伪装的runtime
 */
PRIVATE void start(HashTableNode *node) {
    if (node == NO_NODE) {
        printf("no such a cmd.\n");
        exit(-1);                               /* 子进程退出，退出状态-1 */
    }
    UserTask main = node->userTask;

    char *argv[1];
    sprintf(argv[0], "/%s", node->cmdName);   /* 程序运行时的全限定名 */
    int exitStat = main(1, argv);
    exit(exitStat);
}