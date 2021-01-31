//
// Created by 杜科 on 2021/1/18.
//

#ifndef AOS_STDLIB_H
#define AOS_STDLIB_H

long get_time();

void park(); /* 进程调用后阻塞自己，等待别人unpark */
void unpark(int pid); /* 调用者进程就绪一个进程号为pid的进程，用户进程调用时要求pid>=0 */
int chang_console(int n);

int clean_console();

int get_pid();

int get_ppid();

void delay(unsigned long delay_ms);

int get_alive_proc(char *pids);

#endif //AOS_STDLIB_H
