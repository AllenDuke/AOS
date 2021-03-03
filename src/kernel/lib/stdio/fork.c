//
// Created by 杜科 on 2021/1/13.
//

#include "core/kernel.h"
#include "../include/stdio.h"

PRIVATE bool_t is_one_bit(u8_t num);

/**
 * fork出一条子进程（父进程的一份复制）。
 * @return
 * 成功时，父进程得到子进程的pid，子进程得到0。
 * 失败时，父进程得到-1，子进程创建失败。
 */
PUBLIC int fork() {
    Message msg;
    msg.type = FORK;
    msg.LEVEL = 1;    /* 默认等级为1 */

    send_rec(MM_TASK, &msg);
//    assert(msg.type == SYSCALL_RET); /* assert位于ring0，不能使用 */
//    assert(msg.RETVAL == 0);

    return msg.PID;
}

PRIVATE bool_t is_one_bit(u8_t num) {
    u8_t oneCount = 0;                  /* 按位统计size中1的数量 */
    u8_t tmp = num;
    for (u8_t i = 0; i < LEVEL_BIT; i++) {
        if ((tmp & 1) == 1) {           /* num的第i位为1 */
            oneCount++;
        }
        if (oneCount > 1) return FALSE;
        tmp = tmp >> 1;
    }
    return TRUE;
}

/* fixme 这如果放在fork_level中，会出现异常 */
PRIVATE u8_t levelMap[LEVEL_BIT + 1] = {0, 1, 2, 4, 8, 16, 32, 64, 128};

/**
 * 用于高响应比调度，
 * @param level 它应该是 1~8。最后会被映射成为：
 * 1 -> 1,
 * 2 -> 2,
 * 3 -> 4,
 * 4 -> 8,
 * 5 -> 16,
 * 6 -> 32,
 * 7 -> 64,
 * 8 -> 128
 * 这样映射是为了利用位运算了加快响应比的计算
 * @return
 */
PUBLIC int fork_level(u8_t level) {
    if (level < 1 || level > LEVEL_BIT) return -1;

//    if (level < 1 || level > MAX_LEVEL) return -1;
//    if (!is_one_bit(level)) return -1;

    Message msg;
    msg.type = FORK;
    msg.LEVEL = levelMap[level];
//    msg.LEVEL = level;

//    printf("level:%d\n", msg.LEVEL);

    send_rec(MM_TASK, &msg);
//    assert(msg.type == SYSCALL_RET); /* assert位于ring0，不能使用 */
//    assert(msg.RETVAL == 0);

    return msg.PID;
}