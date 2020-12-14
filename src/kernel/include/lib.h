/**
 * 包含了在操作系统内部调用以访问操作系统其他服务的C库函数原型。
 * 操作系统向外提供的系统调用，也是通过调用这些库函数去实现的。
 */

#ifndef AOS_LIB_H
#define AOS_LIB_H

/* AOS 用户和系统双用库 */
int send_rec(int src, Message *p_msg);
int in_outbox(Message *p_inMsg, Message *p_outMsg);

/* AOS 系统库 */
int send(int dest, Message* p_msg);
int receive(int src, Message* p_msg);


#endif //_FLYANX_SYSLIB_H
