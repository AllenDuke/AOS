//
// Created by 杜科 on 2021/1/12.
//

#include "core/kernel.h"
#include "stdio.h"

PRIVATE void exec_cmd(int cmdLen, char *cmdBuf);

void origin_task() {
    int fd_stdin = open("/dev_tty0", O_RDWR);
    assert(fd_stdin == 0);
    int fd_stdout = open("/dev_tty0", O_RDWR);
    assert(fd_stdout == 1);

    printf("{ORIGIN}->origin_task is working...\n");

    char cmdBuf[128];
    u32_t cmdLen = 0;

    printf("$ ");
    while (1) {
        int r = read(fd_stdin, cmdBuf, 70);
        cmdLen = r;
        cmdBuf[r] = 0;

        int pid = fork();
        if (pid==0) {
            exec_cmd(cmdLen, cmdBuf);
            exit(0);
        }else{
            printf("child pid:%d.\n",pid);
            waitpid(pid);
            printf("child (%d) exited with status: %d.\n", pid, 0);
            printf("$ ");
        }
    }

    while (1) {
        int s;
        int child = waitpid(&s);
        printf("child (%d) exited with status: %d.\n", child, s);
    }

}

PRIVATE void exec_cmd(int cmdLen, char *cmdBuf) {
    if (cmdLen <= 0) return;
    int hash = 0;

    /**
     * 类似java String hash值计算，不直接采取31是因为cmd一般比较短，而31又比较小，
     * 127这个质数比较大，可尽可能地使得hash的高位有值
     * hash=hash*127+cmd[i]
     */
    for (int i = 0; i < cmdLen; i++) {
        hash = ((hash << 8) - hash) + cmdBuf[i];
    }

    /**
     * 扰动函数
     * 同java HashMap hash()
     * hash值的高16位为原hash的高16位，低16位为原hash高16位与原hash低16位异或后的结果。
     * 原因：因为数组的长度为2的次方数，所以最终计算下标时，使用(NR_CMDS-1)&hash，
     * 这样一来，因为(NR_CMDS-1)的高位总是0，hash的高位没有参与到运算，为了进一步降低冲突，应使得高位也参与运算
     * 在权衡质量与速度后，选择了这种方式。成本不高，质量也不错。
     */
    hash = hash ^ (hash >> 16); /* int类型 >> 符号位保持不变 */
    int index = hash & (NR_CMDS - 1);

    char *cmd_map[NR_CMDS] = {
            "",
            "",
            "pwd",
            "date",
            "",
            "",
            "",
            "",
    };

    char *pre = cmd_map[index];
    for (int i = 0; i < cmdLen; i++) {
        if (cmdBuf[i] != *pre || *pre == '\0') {
            printf("no such cmd:%s, hash:%d, index:%d\n", cmdBuf, hash, index);
            return;
        }
        pre++;
    }
    switch (index) {
        case 0: {
            printf("default cmd\n");
            break;
        }
        case 1: {
            printf("default cmd\n");
            break;
        }
        case 2: {
            printf("/\n");
            break;
        }
        case 3: {
            Message msg;
            msg.type = GET_TIME;
            send_rec(CLOCK_TASK, &msg);
            printf("current date is: %d\n", msg.CLOCK_TIME);
            break;
        }
        case 4: {
            printf("default cmd\n");
            break;
        }
        case 5: {
            printf("default cmd\n");
            break;
        }
        case 6: {
            printf("default cmd\n");
            break;
        }
        case 7: {
            printf("default cmd\n");
            break;
        }
    }
}