//
// Created by 杜科 on 2021/1/12.
//

/**
 * 这里的origin进程其实是和内核编译链接在一起的，整个内核的大小就是origin的大小。
 * 实际上它可以变得更小，比如独立编译，然后将其写进硬盘，最后内核将其读到内存里运行。
 *
 * origin的数据尽量在栈上，否则fork后的子进程，访问的数据可能会有异常。所以origin的栈尽可能大一点。
 *
 */
#include "origin.h"
#include "stdio.h"
#include "stdlib.h"

PRIVATE void exec_cmd(CmdResult *result, HashTableNode hashTable[]);

PRIVATE void start(CmdResult *result, HashTableNode *node, HashTableNode table[]);

PRIVATE void split(CmdResult *result, char *cmdBuf, int size);

void origin_task() {

    int fd_stdin = open("/dev_tty0", O_RDWR);
    assert(fd_stdin == 0);
    int fd_stdout = open("/dev_tty0", O_RDWR);
    assert(fd_stdout == 1);

    HashTableNode hashTable[NR_CMD_TSIZE];      /* HashTable，冲突时线性探测，2倍是有想法的 */

    put("pwd", 3, pwd, hashTable);
    put("date", 4, date, hashTable);
    put("echo", 4, echo, hashTable);
    put("cat", 3, cat, hashTable);
    put("touch", 5, touch, hashTable);
    put("vi", 2, vi, hashTable);
    put("rm", 2, rm, hashTable);
    put("clear", 5, clear, hashTable);
    put("proc", 4, proc, hashTable);
    put("ps", 2, ps, hashTable);
    put("top", 3, top, hashTable);
    put("kill", 4, kill, hashTable);
    put("help", 4, help, hashTable);

    printf("{ORIGIN}->origin_task is working...\n");

    char cmdBuf[128];
    int cmdLen = 0;

    printf("$ ");
    while (1) {
        int r = read(fd_stdin, cmdBuf, 70);
        cmdBuf[r] = 0;
        CmdResult cmdResult;
        split(&cmdResult, cmdBuf, r);
        int pid = fork_level(cmdResult.level);
        if (pid == 0) {
            exec_cmd(&cmdResult, hashTable);
        } else {
            if (cmdResult.argv[cmdResult.argc - 1][0] == '&') {
                printf("child pid:%d.\n", pid);
                printf("$ ");
                continue;
            }
            unsigned char exitStat;
//            printf("child pid:%d.\n", pid);
            waitpid_stat(pid, &exitStat);       /* 必定是origin先进入等待状态 */
//            wait();
//            printf("child (%d) exited with status: %d.\n", pid, exitStat);
            printf("$ ");
        }
    }

}

/**
 * 一个伪装的exec命令。
 * 实际上，它应该完成这样的事情，利用文件系统，把命令对应的文件读进内存，然后设置好该进程相关信息。
 */
PRIVATE void exec_cmd(CmdResult *result, HashTableNode hashTable[]) {
    HashTableNode *node = get(result->cmd, result->cmdLen, hashTable);
    start(result, node, hashTable);
}

/**
 * 一个伪装的runtime
 */
PRIVATE void start(CmdResult *result, HashTableNode *node, HashTableNode table[]) {
    if (node == NO_NODE) {
        printf("no such cmd.\n");
        exit(-1);                               /* 子进程退出，退出状态-1 */
    }
    UserTask main = node->userTask;
    if (main == help) {
        result->argv[result->argc++] = table;
    }
    int exitStat = main(result->argc, result->argv);
    exit(exitStat);
}

/**
 * 以空格为分割符，分割cmdBuf，构造真正的命令和参数
 * @param result
 * @param cmdBuf
 * @param size
 */
PRIVATE void split(CmdResult *result, char *cmdBuf, int size) {
    int r = 0;
    while (cmdBuf[r] == ' ' && r < size) r++;   /* 跳过开头的空格 */

    if (r == size) {        /* 只是一串空格 */
        result->cmdLen = 0;
        return;
    }

    int l = r;
    result->cmd = cmdBuf + l;
    while (cmdBuf[r] != ' ' && r < size) r++;       /* 找到了命令的结尾 */
    result->cmdLen = r - l;
    cmdBuf[r++] = 0;                                /* 设置这个空格或者末尾位成为 0 */

    int argL = r;                                   /* main函数的入参 */
    unsigned char level = 1;                        /* 默认的level=1 */

    while (cmdBuf[r] == ' ' && r < size) r++;       /* 跳过空格 */
    /* 如果cmd输入的接下来的一个参数是-l且-l接着一个[1,5]的u8_t，那么它成为level，否则当它们是main函数的入参 */
    if (r + 1 < size && cmdBuf[r] == '-' && cmdBuf[r + 1] == 'l') {
        r += 2;
        while (cmdBuf[r] == ' ' && r < size) r++;   /* 跳过空格 */
        if (r < size) {
            level = cmdBuf[r];
            if (level >= '1' && level <= '5') {
                level = level - '0';
                argL = r + 1;   /* 这里才是main函数的入参起始 */
//                printf("found -l %d.\n",result->level);
            }
        }
//        printf("found -l.\n");
    }

    result->level = level;

    r = argL;

    /* argv至少有一个参数，argv[0]为程序运行时的全限定名 */
    sprintf(result->argv[0], "/%s", result->cmd);
    result->argc = 1;

    /* 开始专心处理参数 */
    while (r < size) {
        while (cmdBuf[r] == ' ' && r < size) r++;   /* 跳过空格 */
        if (r >= size) {        /* 这里要用>= */
            return;
        }
        result->argv[result->argc] = cmdBuf + r;
        result->argc++;
        while (cmdBuf[r] != ' ' && r < size) r++;
        cmdBuf[r++] = 0;        /* 设置这个空格或者末尾位成为 0 */
    }

    return;
}