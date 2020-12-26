//
// Created by 杜科 on 2020/10/8.
//

#include "core/kernel.h"


/* 系统进程表，包含系统任务以及系统服务 */
SysProc sysProcs[] = {
        { tty_task, TTY_TASK_STACK, "TTY" },
        { clock_task, CLOCK_TASK_STACK, "CLOCK" },
        { mm_task,MM_TASK_STACK,"MM"},
        { idle_task, IDLE_TASK_STACK, "IDLE" },
        { 0, HARDWARE_STACK, "HARDWARE" },/* 虚拟硬件任务，只是占个位置 - 用作判断硬件中断 */
};

void aos_main(void) {

    kprintf("aos_main\n");

//    int i=1/0; /* 除法错误正常 */

    /**
     * 进程表的所有表项都被标志为空闲;
     * 对用于加快进程表访问的 p_proc_addr 数组进行初始化。
     */
    register Process *p_proc;
    register int logicNum;
    for(p_proc = BEG_PROC_ADDR, logicNum = -NR_TASKS; p_proc < END_PROC_ADDR; p_proc++, logicNum++) {
        if(logicNum > 0)    /* 系统服务和用户进程 */
            strcpy(p_proc->name, "unused");
        p_proc->logicNum = logicNum; /* 系统服务的逻辑号从-NR_TASKS到-1 */
        p_proc->pid=logicNum;
        gp_procs[logic_nr_2_index(logicNum)] = p_proc;
        
    }

    /**
     * 初始化多任务支持
     * 为系统任务和系统服务设置进程表，它们的堆栈被初始化为数据空间中的数组
     */
    SysProc *p_sysProc;
    reg_t sysProcStackBase = (reg_t) sysProcStack;
    u8_t  privilege;        /* CPU 权限 */
    u8_t rpl;               /* 段访问权限 */
    for(logicNum = -NR_TASKS; logicNum <= NR_LAST_TASK; logicNum++) {   /* 遍历整个系统任务 */
        p_proc = proc_addr(logicNum);                                 /* 拿到系统任务对应应该放在的进程指针 */
        p_sysProc = &sysProcs[logic_nr_2_index(logicNum)];     /* 系统进程项 */
        strcpy(p_proc->name, p_sysProc->name);                         /* 拷贝名称 */
        /* 判断是否是系统任务 */
        if (logicNum < 0) {  /* 系统任务 */
            if (p_sysProc->stackSize > 0) {
                /* 如果任务存在堆栈空间，设置任务的堆栈保护字，所以栈的大小要适当，防止越界修改了重要数据 */
                p_proc->stackGuardWord = (reg_t *) sysProcStackBase;
                *p_proc->stackGuardWord = SYS_TASK_STACK_GUARD;
            }
            /* 设置权限 */
            p_proc->priority = PROC_PRI_TASK;
            rpl = privilege = TASK_PRIVILEGE;
        }
        /* 堆栈基地址 + 分配的栈大小 = 栈顶，此时又成为了下一个进程栈顶 */
        sysProcStackBase += p_sysProc->stackSize;

        /* ================= 初始化系统进程的 LDT ================= */
        /**
         * 所有的系统任务和服务，其段基地址都是0。
         * 而所有的用户进程，因为不采取分页机制，所以其地址必不为0，得设置为分配的空间的基地址。
         *
         * 在linux中所有进程共享一个ldt，也因为linux采用的基于分页的虚拟内存管理机制，
         */
        p_proc->ldt[CS_LDT_INDEX] = g_gdt[TEXT_INDEX];
        p_proc->ldt[DS_LDT_INDEX] = g_gdt[DATA_INDEX];
        /* ================= 改变DPL描述符特权级 ================= */
        p_proc->ldt[CS_LDT_INDEX].access = (DA_CR | (privilege << 5));
        p_proc->ldt[DS_LDT_INDEX].access = (DA_DRW | (privilege << 5));
        /* 设置任务和服务的内存映射 */
        p_proc->map.base = KERNEL_TEXT_SEG_BASE;
        p_proc->map.size = 0;     /* 内核的空间是整个内存，所以设置它没什么意义，为 0 即可 */

        /**
         * 初始化系统进程的栈帧以及上下文环境，设置TI=1，表示访问的是LDT。
         * LDT从0起，第0个是代码段描述符，第1个是数据段描述符，所以
         * cs=0，ds=8。注意区别与GDT的访问形式。
         */
        p_proc->regs.cs = ((CS_LDT_INDEX * DESCRIPTOR_SIZE) | SA_TIL | rpl); /* cs应=0 设置TI=1，表示访问的是LDT */
        p_proc->regs.ds = ((DS_LDT_INDEX * DESCRIPTOR_SIZE) | SA_TIL | rpl); /* ds应=8 */
        p_proc->regs.es = p_proc->regs.fs = p_proc->regs.ss = p_proc->regs.ds;  /* C 语言不加以区分这几个段寄存器 */
        p_proc->regs.gs = ((KERNEL_GS_SELECTOR | rpl) & SA_RPL_MASK);     /* gs 指向显存 */
        p_proc->regs.eip = (reg_t) p_sysProc->task;                        /* eip 指向要执行的代码首地址 */
        p_proc->regs.esp = sysProcStackBase;                           /* 设置栈顶 */
        p_proc->regs.eflags = is_task_proc(p_proc) ? INIT_TASK_PSW : INIT_PSW; /* 设置if位 */

        /* 进程刚刚初始化，让它处于可运行状态，所以标志位上没有1 */
        p_proc->flags = CLEAN_MAP;

        /* 如果该进程不是待机任务 或 虚拟硬件，就绪它 */
        if(!is_idle_hardware(logicNum)) ready(p_proc);
    }

    /* 设置消费进程，它需要一个初值。因为系统闲置刚刚启动，所以此时闲置进程是一个最合适的选择。
 * 随后在调用下一个函数 lock_hunter 进行第一次进程狩猎时可能会选择其他进程。
 */
    gp_billProc = proc_addr(IDLE_TASK);
    proc_addr(IDLE_TASK)->priority = PROC_PRI_IDLE;
    lock_hunter();      /* 让我们看看，有什么进程那么幸运的被抓出来第一个执行 */

    /* 最后,main 的工作至此结束。它的工作到初始化结束为止。restart 的调用将启动第一个任务，
     * 控制权从此不再返回到main。
     *
     * restart 作用是引发一个上下文切换,这样 curr_proc 所指向的进程将运行。
     * 当 restart 执行了第一次时,我们可以说 AOS 正在运行-它在执行一个进程。
     * restart 被反复地执行,每当系统任务、服务器进程或用户进程放弃运行机会挂
     * 起时都要执行 restart,无论挂起原因是等待输入还是在轮到其他进程运行时将控制权转交给它们。
     */
    restart();
}

