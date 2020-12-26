//
// Created by 杜科 on 2020/12/18.
//

#ifndef AOS_MESSAGE_H
#define AOS_MESSAGE_H

/* 定义6种消息域将使得更易于在不同的体系结构上重新编译。 */
typedef struct {int m1i1, m1i2, m1i3; char *m1p1, *m1p2, *m1p3;} mess_union1;
typedef struct {int m2i1, m2i2, m2i3; long m2l1, m2l2; char *m2p1;} mess_union2;
typedef struct {int m3i1, m3i2; char *m3p1; char m3ca1[M3_STRING];} mess_union3;
typedef struct {long m4l1, m4l2, m4l3, m4l4, m4l5;} mess_union4;
typedef struct {char m5c1, m5c2; int m5i1, m5i2; long m5l1, m5l2, m5l3;}mess_union5;
typedef struct {int m6i1, m6i2, m6i3; long m6l1; sighandler_t m6f1;} mess_union6;

/* *
 * 消息，AOS中的进程通信的根本，同时也是客户端和服务端通信的根本
 * 此数据结构来源自MINIX
 */
typedef struct message_s{
    int source;         /* 谁发送的消息 */
    int type;           /* 消息的类型，用于判断告诉对方意图 */
    union {             /* 消息域，一共可以是六种消息域类型之一 */
        mess_union1 m_u1;
        mess_union2 m_u2;
        mess_union3 m_u3;
        mess_union4 m_u4;
        mess_union5 m_u5;
        mess_union6 m_u6;
    } m_u;
} Message;

/* 以下定义提供了消息中消息域有用成员的简短名称。 */
/* 消息域1中的消息属性 */
#define m1_i1   m_u.m_u1.m1i1
#define m1_i2   m_u.m_u1.m1i2
#define m1_i3   m_u.m_u1.m1i3
#define m1_p1   m_u.m_u1.m1p1
#define m1_p2   m_u.m_u1.m1p2
#define m1_p3   m_u.m_u1.m1p3

/* 消息域2中的消息属性 */
#define m2_i1   m_u.m_u2.m2i1
#define m2_i2   m_u.m_u2.m2i2
#define m2_i3   m_u.m_u2.m2i3
#define m2_l1   m_u.m_u2.m2l1
#define m2_l2   m_u.m_u2.m2l2
#define m2_p1   m_u.m_u2.m2p1

/* 消息域3中的消息属性 */
#define m3_i1   m_u.m_u3.m3i1
#define m3_i2   m_u.m_u3.m3i2
#define m3_p1   m_u.m_u3.m3p1
#define m3_ca1  m_u.m_u3.m3ca1

/* 消息域4中的消息属性 */
#define m4_l1   m_u.m_u4.m4l1
#define m4_l2   m_u.m_u4.m4l2
#define m4_l3   m_u.m_u4.m4l3
#define m4_l4   m_u.m_u4.m4l4
#define m4_l5   m_u.m_u4.m4l5

/* 消息域5中的消息属性 */
#define m5_c1   m_u.m_u5.m5c1
#define m5_c2   m_u.m_u5.m5c2
#define m5_i1   m_u.m_u5.m5i1
#define m5_i2   m_u.m_u5.m5i2
#define m5_l1   m_u.m_u5.m5l1
#define m5_l2   m_u.m_u5.m5l2
#define m5_l3   m_u.m_u5.m5l3

/* 消息域6中的消息属性 */
#define m6_i1   m_u.m_u6.m6i1
#define m6_i2   m_u.m_u6.m6i2
#define m6_i3   m_u.m_u6.m6i3
#define m6_l1   m_u.m_u6.m6l1
#define m6_f1   m_u.m_u6.m6f1

#define m_exec_name     mmsg_in.m1_p1           /* 要执行的文件名 */
#define m_exec_nlen     mmsg_in.m1_i1           /* 要执行的文件名长度 */
#define m_pid		    mmsg_in.m1_i1           /* 进程号 */
#define m_status		mmsg_in.m1_i1           /* 状态码 */
#define m_sig_nr		mmsg_in.m1_i2           /* 信号 */
#define m_stack_ptr     mmsg_in.m1_p2           /* 栈指针 */
#define m_stack_bytes   mmsg_in.m1_i2           /* 栈大小 */

/* 以下名称是应答消息中变量的同义词。 */
#define reply_rs1       reply.type      /* 回复结果1 */
#define reply_rs2	    reply.m2_i1     /* 回复结果2 */
#define reply_ptr	    reply.m2_p1     /* 回复指针 */
#define reply_mask	    reply.m2_l1     /* 回复掩码 */

#endif //AOS_MESSAGE_H