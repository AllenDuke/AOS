//
// Created by 杜科 on 2020/12/23.
//

#ifndef AOS_MM_H
#define AOS_MM_H

/**
 * 二叉堆的形式
 * 一个节点管理一段连续的页，这里以页为单位 ，比用树的形式减少12字节的指针大小，
 * 同时借鉴了伙伴系统的内存管理机制
 */
typedef struct card_node_s {        /* 可能对齐，12字节*/
    phys_page base;
    phys_page len;
    bool_t available;               /* TRUE为辖下可用，可全部借出，FALSE为不可全部借出 */
} CardNode;

#define NIL_CARD_NODE (CardNode*) 0

typedef struct mm_process_s {
    MemoryMap map;                  /* 参考内核中的进程结构map */
    u8_t exit_status;               /* 退出状态：在进程已经结束而父进程还没有执行对它的WAIT时的终止状态。 */
    pid_t pid;                      /* 进程pid */
    u8_t aliveChildCount;           /* 存活的子进程的数量，在父fork时增加，子exit时减少 */
    pid_t ppid;                     /* 父进程的进程pid，空闲时为NO_TASK */
    u16_t flags;                    /* 标志 */
    char name[32];
    phys_page keep;                 /* 持有的页面数 */
} MMProcess;

/* 标志值 */
#define IN_USE              0x001	/* 当进程插槽在使用时被设置 */
#define WAITPID             0x002	/* 当waitpid（等待）系统调用时被设置 */
#define ZOMBIE              0x004	/* 当EXIT时被置位，当WAIT时被复位清除。 */
#define PAUSED              0x008	/* set by PAUSE system call */
#define ALARM_ON            0x010	/* set when SIGALRM timer started */
#define WAITPID_STAT        0x020	/* 当waitpid_stat（等待）系统调用时被设置 */
#define	TRACED		        0x040	/* set if process is to be traced */
#define STOPPED		        0x080	/* 设置当进程停止跟踪调试 */
#define SIG_SUSPENDED 	    0x100	/* 系统调用SIGSUSPEND */
#define WAIT_F	 	        0x200	/* 当wait（等待）系统调用时被设置 */
#define WAITING             0x222   /*  waiting状态*/
#define ON_SWAP	 	        0x400	/* 数据段被换出 */
#define SWAP_IN	 	        0x800	/* set if on the "swap this in" queue */

#define NIL_MMPROC ((MMProcess *) 0)

#define WHANG           0     /* 调用者需要等待 */
#define WNOHANG         1   /* 不需要等待子进程退出 */
#define WUNTRACED       2   /* 为了任务控制；但我还未实现 */

#endif //AOS_MM_H
