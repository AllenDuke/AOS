//
// Created by 杜科 on 2020/10/15.
//
/**
 * 一些结构体类型
 */
#ifndef AOS_STRUCT_TYPE_H
#define AOS_STRUCT_TYPE_H


/* 引导参数 */
typedef struct boot_params_s{
    u32_t memory_size;          /* 内存大小 */
    phys_addr kernel_file;     /* 内核所在绝对物理地址 */
} BootParam;

/**
 * StackFrame定义了如何将寄存器值保存到堆栈上的数据结构
 * 这个结构非常重要-在进程被投入运行状态或被脱开运行状态时,它被用来保
 * 存和恢复CPU的内部状态。将其定义成可以用汇编语言高速读写的格式,这将
 * 减少进程上下文切换的时间，进程指针必须指向这里。
 * 严格要求顺序
 */
typedef struct stackframe_s{
    /* 低地址 */
    /* =========== 所有的特殊段寄存器，我们手动压入 =========== */
    reg_t	gs;
    reg_t	fs;
    reg_t	es;
    reg_t	ds;
    /* ============== 所有的普通寄存器，我们通过 pushad 手动压入 ============== */
    reg_t	edi;
    reg_t	esi;
    reg_t	ebp;
    reg_t	kernel_esp;	/* pushad 压入的 esp，这个时候已经自动从低特权级到了 0 特权级，
                         * 所以这个其实是从tss.esp0！而 popad 指令也会忽略这个，不会
                         * 恢复它。
                         */
    reg_t	ebx;
    reg_t	edx;
    reg_t	ecx;
    reg_t	eax;
    /* ======= call save()自动保存的返回地址 ======= */
    reg_t	ret_addr;
    /* ============ 中断自动压入的内容 ============= */
    reg_t	eip;        /* 中断门和调用门有一点点不同，那就是中断门还会压入一个eflags */
    reg_t	cs;
    reg_t	eflags;     /* 中断门自动压入 */
    reg_t	esp;
    reg_t   ss;
    /* =========================================== */
    /* 高地址 */
} StackFrame;

/* 内存映像
 * 这个结构能够描述一个内存块信息
 */
typedef struct memory_map{
    phys_addr base;    /* 这块内存的基地址 */
    phys_addr size;    /* 这块内存有多大？ */
} MemoryMap;


typedef void (*SysTask) (void);
typedef void (*WatchDog) (void);
/**
 * 系统进程表项定义
 * 一个表项可以存放一个系统级别的进程，在这里我们和用户进程表项分开定义了
 * 因为它们特权级不同，待遇也不同，就这个理解就应该让我们区别对待。
 */
typedef struct sys_proc {
    SysTask task;           /* 这是一个函数指针，指向实际要执行的任务 */
    int     stack_size;     /* 系统进程的栈大小 */
    char    name[16];       /* 系统进程名称 */
} SysProc;

/* 定义6种消息域将使得更易于在不同的体系结构上重新编译。 */
typedef struct {int m1i1, m1i2, m1i3; char *m1p1, *m1p2, *m1p3;} mess_union1;
typedef struct {int m2i1, m2i2, m2i3; long m2l1, m2l2; char *m2p1;} mess_union2;
typedef struct {int m3i1, m3i2; char *m3p1; char m3ca1[M3_STRING];} mess_union3;
typedef struct {long m4l1, m4l2, m4l3, m4l4, m4l5;} mess_union4;
typedef struct {char m5c1, m5c2; int m5i1, m5i2; long m5l1, m5l2, m5l3;}mess_union5;
typedef struct {int m6i1, m6i2, m6i3; long m6l1; sighandler_t m6f1;} mess_union6;

/* 消息，AOS中的进程通信的根本，同时也是客户端和服务端通信的根本
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

#endif //AOS_STRUCT_TYPE_H
