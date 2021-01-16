//
// Created by 杜科 on 2021/1/13.
//

#include "core/kernel.h"
#include "../include/stdio.h"

 /**
  * fork出一条子进程（父进程的一份复制）。
  * @return
  * 成功时，父进程得到子进程的pid，子进程得到0。
  * 失败时，父进程得到-1，子进程创建失败。
  */
PUBLIC int fork() { // todo 增加优先级参数
    Message msg;
    msg.type = FORK;

    send_rec(MM_TASK, &msg);
//    assert(msg.type == SYSCALL_RET); /* assert位于ring0，不能使用 */
//    assert(msg.RETVAL == 0);

    return msg.PID;
}