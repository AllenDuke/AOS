//
// Created by 杜科 on 2020/11/2.
//

#ifndef AOS_PROCESS_H
#define AOS_PROCESS_H

/**
 * 进程，一个进程是包含 进程的寄存器信息(栈帧) 和 LDT(本地描述符表) 以及 自定义的一些属性
 * CPU 通过 PCB来调度进程，而不是通过tts
 * 各成员的位置是有讲究的，不能随便放置。
 */
typedef struct process_s{
    /* 这里存放 进程的寄存器信息(栈帧) 和 LDT(本地描述符表) 信息，它们由 CPU 使用并调度，和硬件相关 */
    StackFrame regs;                /* 进程的栈帧，包含进程自己所有寄存器的信息 */
    reg_t ldtSelector;              /* 进程的 LDT 选择子，是LDT指针描述符在GDT中的偏移量，用于lldt指令 */
    SegDescriptor ldt[LDT_SIZE];    /* 进程有两个段，代码段0和数据段1(这里与堆栈段共用) */

    /* 从这后面都是用户自定义的属性，和硬件无关 */

    /**
     * 堆栈保护字指针，指向堆栈保护字
     * 用于识别堆栈是否正常，如果被改变那么堆栈已经出现问题
     */
    reg_t* stackGuardWord;

    /**
     * 进程的内存映像
     * 现在包括正文段和数据段（堆栈段），但只有一个而非一个数组，
     * 因为 AOS 当前版本并不加以细分这几个段，它们都使用同一个段区域，所以一个就可以描述了。
     */
    MemoryMap map;

    /* 调度相关 */
    /**
     * flags 状态标志位图域中的标志位标识进程的状态。如果其中任一位被置位,则进程将被堵塞
     * 无法运行。各种标志被定义和描述请往下看，如果该进程表项未被使用,则P_SLOT_FREE被置位。
     */
    u8_t flags;
    pid_t pid;                      /* 进程号，用户可见的 */
    u8_t priority;                  /* 权限：任务0/服务1/用户进程3 */
    struct process_s* p_nextReady;  /* 指向下一个就绪的进程，形成一个队列 */
    int logicNum;                   /* 进程在进程表中的逻辑编号，主要用于表中的进程快速访问 */
    bool_t intBlocked;              /* 被置位，当目标进程有一条中断消息被繁忙的任务堵塞了 */
    bool_t intHeld;                 /* 被置位，当目标进程有一条中断消息被繁忙的系统调用挂起保留了 */
    struct process_s* p_nextHeld;   /* 被挂起保留的中断过程队列 */

    /* 时间相关 */
    clock_t userTime;               /* 用户时间(以时钟滴答为单位)，即进程自己使用的时间 */
    clock_t sysTime;                /* 系统时间(以时钟滴答为单位)，即进程调用了系统任务的时间，或者说进程本身就是系统任务 */
    clock_t childUserTime;          /* 子进程累积使用的用户时间 */
    clock_t childSysTime;           /* 子进程累积使用的系统时间 */
    clock_t alarm;                  /* 进程下一次闹钟响起的时间 */

    /* 消息通信相关 */
    Message *inBox;                 /* 收件箱，当有人发送消息过来将被邮局将消息放在这里，它是一个物理地址 */
    Message *outBox;                /* 发件箱，当一个进程发送消息给另一个进程，邮局将会从这里获取要发送的消息，它也是一个物理地址 */
    Message *transfer;              /* 存放中转消息，物理地址 */
    /* 当一个进程执行接收操作，但没有发现有任何人想发消息过来时将会堵塞，然后将自己期望接收消息的进程逻辑编号保存在这 */
    int getFrom;
    int sendTo;                     /* 同上，保存要发送消息给谁？ */
    struct process_s* p_nextWaiter; /* 指向下一个要发送消息给我的人，为了实现等待队列 */

    char name[32];                  /* 这个没啥好说的，就是进程的名称，记得起个好名字哦 */
} Process;


/* 系统堆栈的保护字 */
#define SYS_TASK_STACK_GUARD	((reg_t) (sizeof(reg_t) == 2 ? 0xBEEF : 0xDEADBEEF))    /* 任务的 */
#define SYS_SERVER_STACK_GUARD	((reg_t) (sizeof(reg_t) == 2 ? 0xBFEF : 0xDEADCEEF))    /* 服务的 */

/**
 * flags 状态标志位图域中的标志位状态定义
 * 现在一共有三种状态，只要任一状态位被置位，那么进程就会被堵塞
 */
#define CLEAN_MAP       0       /* 干净的状态，进程正在快乐的执行 */
#define NO_MAP		    0x01	/* 执行一个 FORK 操作后,如果子进程的内存映像尚未建立起来,那么 NO_MAP 将被置位以阻止子进程运行 */
#define SENDING		    0x02	/* 进程正在试图发送一条消息 */
#define RECEIVING	    0x04	/* 进程正在试图接收一条消息 */
#define PENDING		    0x08	/* set when inform() of signal pending */
#define SIG_PENDING	    0x10	/* keeps to-be-signalled proc from running */
#define PROC_STOP		0x20	/* set when process is being traced */

/* 进程权限定义 */
#define PROC_PRI_NONE	0	    /* 表示该进程插槽未使用 */
#define PROC_PRI_TASK	1	    /* 部分内核，即系统任务 */
#define PROC_PRI_SERVER	2	    /* 内核之外的系统服务 */
#define PROC_PRI_USER	3	    /* 用户进程 */
#define PROC_PRI_IDLE	4	    /* 空闲进程，一个特殊的进程，当系统没有正在活动的进程时被运行 */

/* 对过程表地址操作的一些宏定义。 */
#define BEG_PROC_ADDR       (&g_procs[0])
#define END_PROC_ADDR       (&g_procs[NR_TASKS + NR_PROCS])
#define END_TASK_ADDR       (&g_procs[NR_TASKS])
#define BEG_SERVER_ADDR     (&g_procs[NR_TASKS])
#define BEG_USER_PROC_ADDR  (&g_procs[NR_TASKS +NR_LAST_TASK])

/* 下面的这些宏能帮助我们快速做一些进程判断等简单的工作 */
#define NIL_PROC                ((Process *) 0)       /* 空进程指针 */
#define logic_nr_2_index(n)     (NR_TASKS + n)
#define is_idle_hardware(n)     ((n) == IDLE_TASK || (n) == HARDWARE)      /* 是空闲进程 或 硬件（特殊进程）？ */
#define is_ok_proc_nr(n)        ((unsigned) ((n) + NR_TASKS) < NR_PROCS + NR_TASKS)   /* 进程索引号是否合法 */
#define is_ok_src_dest(n)       (is_ok_proc_nr(n) || (n) == ANY)            /* 是个合法的发送或接收进程？ */
#define is_any_hardware(n)      ((n) == ANY || (n) == HARDWARE)             /* 发送/接收进程是任何 或 硬件（特殊进程）？ */
#define is_empty_proc(p)        ((p)->priority == PROC_PRI_NONE)            /* 是个空进程？ */
#define is_sys_proc(p)          ((p)->priority != PROC_PRI_USER)            /* 是个系统进程？ */
#define is_task_proc(p)         ((p)->priority == PROC_PRI_TASK)            /* 是个系统任务进程？ */
#define is_serv_proc(p)         ((p)->priority == PROC_PRI_SERVER)          /* 是个系统服务进程？ */
#define is_user_proc(p)         ((p)->priority == PROC_PRI_USER)            /* 是个用户进程？ */

/**
 * 提供宏 proc_addr 是因为在C语言中下标不能为负数。在逻辑上,数组proc应从 -NR_TASKS到+NR_PROCS(因为用户进程从1开始)。
 * 但在C语言中下标必须从0开始,所以proc[0]指向进程表项下标最小的任务,其他也依次类推。为了更便于记录进程表项与
 * 进程之间的对应关系,我们可以使用
 * proc = proc_addr(n);
 * 将进程n的进程表项地址赋给rp,无论它是正还是负。
 */
#define proc_addr(n)      (gp_procs + NR_TASKS)[(n)]                        /* 得到进程的指针 */
#define cproc_addr(n)     (&(g_procs + NR_TASKS)[(n)])                      /* 得到进程的地址 */
#define proc_vir2phys(p, vir) ((phys_addr)(p)->map.base + (vir_addr)(vir))  /* 进程内的虚拟地址转物理地址 */


/* 互斥锁，信号量为1 */
typedef struct mutex_s{

}Mutex;

#endif //AOS_PROCESS_H
