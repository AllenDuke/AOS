//
// Created by 杜科 on 2020/10/8.
//

#include "../include/core/kernel.h"


/* === 系统进程表，包含系统任务以及系统服务 === */
SysProc sys_proc_table[] = {
        /* ************************* 系统任务 ************************* */
        /* 待机任务 */
        { idle_task, IDLE_TASK_STACK, "IDLE" },
        /* ************************* 系统服务 ************************* */
};

void aos_main(void) {

    printf("aos_main\n");

//    int i=1/0; /* 除法错误正常 */

    clock_task();

    /**
     * 进程表的所有表项都被标志为空闲;
     * 对用于加快进程表访问的 p_proc_addr 数组进行初始化。
     */
    register Process *proc;
    register int logic_nr;
    for(proc = BEG_PROC_ADDR, logic_nr = -NR_TASKS; proc < END_PROC_ADDR; proc++, logic_nr++) {
        if(logic_nr > 0)    /* 系统服务和用户进程 */
            strcpy(proc->name, "unused");
        proc->logic_nr = logic_nr; /* 系统服务的逻辑号从-NR_TASKS到-1 */
        p_proc_addr[logic_nr_2_index(logic_nr)] = proc;
    }

    /**
     * 初始化多任务支持
     * 为系统任务和系统服务设置进程表，它们的堆栈被初始化为数据空间中的数组
     */
    SysProc *sys_proc;
    reg_t sys_proc_stack_base = (reg_t) sys_proc_stack;
    u8_t  privilege;        /* CPU 权限 */
    u8_t rpl;               /* 段访问权限 */
    for(logic_nr = -NR_TASKS; logic_nr <= LOW_USER; logic_nr++) {   /* 遍历整个系统任务 */
        proc = proc_addr(logic_nr);                                 /* 拿到系统任务对应应该放在的进程指针 */
        sys_proc = &sys_proc_table[logic_nr_2_index(logic_nr)];     /* 系统进程项 */
        strcpy(proc->name, sys_proc->name);                         /* 拷贝名称 */
        /* 判断是否是系统任务还是系统服务 */
        if (logic_nr < 0) {  /* 系统任务 */
            if (sys_proc->stack_size > 0) {
                /* 如果任务存在堆t栈空间，设置任务的堆栈保护字 */
                proc->stack_guard_word = (reg_t *) sys_proc_stack_base;
                *proc->stack_guard_word = SYS_TASK_STACK_GUARD;
            }
            /* 设置权限 */
            proc->priority = PROC_PRI_TASK;
            rpl = privilege = TASK_PRIVILEGE;
        } else {            /* 系统服务 */
            if (sys_proc->stack_size > 0) {
                /* 如果任务存在堆栈空间，设置任务的堆栈保护字 */
                proc->stack_guard_word = (reg_t *) sys_proc_stack_base;
                *proc->stack_guard_word = SYS_SERVER_STACK_GUARD;
            }
            proc->priority = PROC_PRI_SERVER;
            rpl = privilege = SERVER_PRIVILEGE;
        }
        /* 堆栈基地址 + 分配的栈大小 = 栈顶，此时又成为了下一个进程栈顶 */
        sys_proc_stack_base += sys_proc->stack_size;

        /* ================= 初始化系统进程的 LDT ================= */
        proc->ldt[CS_LDT_INDEX] = g_gdt[TEXT_INDEX];  /* 这里是深拷贝 和内核公用段 */
        proc->ldt[DS_LDT_INDEX] = g_gdt[DATA_INDEX];
        /* ================= 改变DPL描述符特权级 ================= */
        proc->ldt[CS_LDT_INDEX].access = (DA_CR | (privilege << 5));
        proc->ldt[DS_LDT_INDEX].access = (DA_DRW | (privilege << 5));
        /* 设置任务和服务的内存映射 */
        proc->map.base = KERNEL_TEXT_SEG_BASE;
        proc->map.size = 0;     /* 内核的空间是整个内存，所以设置它没什么意义，为 0 即可 */
        /* ================= 初始化系统进程的栈帧以及上下文环境 ================= */
        proc->regs.cs = ((CS_LDT_INDEX * DESCRIPTOR_SIZE) | SA_TIL | rpl);
        proc->regs.ds = ((DS_LDT_INDEX * DESCRIPTOR_SIZE) | SA_TIL | rpl);
        proc->regs.es = proc->regs.fs = proc->regs.ss = proc->regs.ds;  /* C 语言不加以区分这几个段寄存器 */
        proc->regs.gs = (KERNEL_GS_SELECTOR & SA_RPL_MASK | rpl);       /* gs 指向显存 */
        proc->regs.eip = (reg_t) sys_proc->task;                        /* eip 指向要执行的代码首地址 */
        proc->regs.esp = sys_proc_stack_base;                           /* 设置栈顶 */
        proc->regs.eflags = is_task_proc(proc) ? INIT_TASK_PSW : INIT_PSW; /* 设置if位 */

        /* 进程刚刚初始化，让它处于可运行状态，所以标志位上没有1 */
        proc->flags = CLEAN_MAP;
    }
    /* 启动 A */
    gp_curProc = proc_addr(-1);
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


/*=========================================================================*
 *				idle_task				   *
 *	            待机任务
 *=========================================================================*/
PUBLIC void idle_task(void) {
    /* 本例程是一个空循环，AOS 系统没有任何进程就绪时，则会调用本例程。
     * 本例程的循环体使用了 hlt 指令，使其 CPU 暂停工作并处于待机等待状态，
     * 不至于像传统的死循环一样，消耗大量的 CPU 资源。而且在每个待机的过程
     * 中都会保持中断开启，保证待机时间内随时可以响应活动。
     */
    printf("idle...\n");
    while (TRUE)
        level0(halt);
}
