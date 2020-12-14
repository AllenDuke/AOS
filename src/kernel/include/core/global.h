/**
 * 内核所需要的全局变量
 */

#ifndef AOS_GLOBAL_H
#define AOS_GLOBAL_H

/* 全局描述符表GDT */
SegDescriptor g_gdt[GDT_SIZE];

/* 任务状态段TSS(Task-State Segment) 104字节 */
TSS g_tss;          /* 用于内核任务 */

u8_t gp_gdt[6];     /* GDT指针，0~15：Limit 16~47：Base */
u8_t gp_idt[6];     /* IDT指针，同上 */

int g_dispPosition; /* low_print函数需要它标识显示位置 */

irq_handler g_irqHandlers[NR_IRQ_VECTORS];  /* 中断处理函数 */

/* 内核内存 */
MemoryMap g_kernelMap;  /* 内核内存映像 */

/* 多进程相关 */
struct process_s *gp_curProc;   /* 当前正在运行的进程 */

Process g_procs[NR_TASKS + NR_SERVERS + NR_PROCS];   /* 进程表，记录系统所有进程 */
/**
 * 因为进程表的访问非常频繁,并且计算数组中的一个地址需要用到很慢的乘法操作，
 * 所以使用一个指向进程表项的指针数组p_proc_addr 来加快操作速度
 */
Process* gp_procs[NR_TASKS + NR_SERVERS + NR_PROCS];

/**
 * bill_proc指向正在对其CPU使用计费的进程。当一个用户进程调用文件系统,而文件系统正在运行
 * 时,curr_proc(在global.h中)指向文件系统进程,但是bill_proc将指向发出该调用的用户进程。因为文件系统使用的
 * CPU时间被作为调用者的系统时间来计费。
 */
Process* gp_billProc;

/**
 * 两个数组ready_head和ready_tail用来维护调度队列。例如,ready_head[TASK_Q]指向就绪任务队列中的第一个进程。
 * 就绪进程队列一共分为三个
 * ready_head[TASK_QUEUE]：  就绪系统任务队列
 * ready_head[SERVER_QUEUE]：就绪服务进程队列
 * ready_head[USER_QUEUE]：  就绪用户进程队列
 * 再举个例子，我们需要拿到用户进程队列的第3个进程，则应该这么拿：ready_head[USER_QUEUE]->next_ready->next_ready，简单吧？
 */
Process* gp_readyHeads[NR_PROC_QUEUE];
Process* gp_readyTails[NR_PROC_QUEUE];

struct process_s *gp_heldHead;    /* 中断挂起队列头指针 */
struct process_s *gp_heldTail;    /* 中断挂起队列尾指针 */

u8_t kernelReenter;            /* 记录内核中断重入次数 */

/* 所有系统进程堆栈的堆栈空间。 （声明为（char *）使其对齐。） */
char* sysProcStack[TOTAL_TASK_STACK / sizeof(char *)];

TTY ttys[NR_CONSOLES];
CONSOLE consoles[NR_CONSOLES];
int nrCurConsole;     /* 表示当前的控制台 */

/* 其他 */
BootParam *gp_bootParam;    /* 引导参数指针 */
aos_syscall level0Fn;    /* 提权成功的函数指针放在这里 */

#endif // AOS_GLOBAL_H
