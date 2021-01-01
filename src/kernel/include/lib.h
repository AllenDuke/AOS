/**
 * 包含了在操作系统内部调用以访问操作系统其他服务的C库函数原型。
 * 操作系统向外提供的系统调用，也是通过调用这些库函数去实现的。
 */

#ifndef AOS_LIB_H
#define AOS_LIB_H

/* AOS 用户和系统双用库 */
int send_rec(int src, Message *p_msg);
int in_outbox(Message *p_inMsg, Message *p_outMsg);
void park(); /* 进程调用后阻塞自己，等待别人unpark */
void unpark(int pid); /* 调用者进程就绪一个进程号为pid的进程，用户进程调用时要求pid>=0 */

/* AOS 系统库 */
int send(int dest, Message* p_msg);
int receive(int src, Message* p_msg);

#endif //AOS_LIB_H
