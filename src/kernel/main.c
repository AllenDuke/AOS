//
// Created by 杜科 on 2020/10/8.
//

#include "core/kernel.h"

/* 系统进程表，包含系统任务以及系统服务 */
SysProc sysProcs[] = {
#ifdef ENABLE_TEST
        //        { fs_test, TEST_TASK_STACK, "TEST" },
                { tty_test, TEST_TASK_STACK, "TEST" },
#endif
        {tty_task, TTY_TASK_STACK, "TTY"},
        {at_winchester_task, HD_TASK_STACK, "HD"},// todo 交换hd和clock的位置会导致tty堆栈保护字被破坏，为什么？
        {fs_task, FS_TASK_STACK, "FS"},
        {clock_task, CLOCK_TASK_STACK, "CLOCK"},
        {mm_task, MM_TASK_STACK, "MM"},
        {idle_task, IDLE_TASK_STACK, "IDLE"},
        {0, HARDWARE_STACK, "HARDWARE"},/* 虚拟硬件任务，只是占个位置 - 用作判断硬件中断 */
};

PRIVATE void init_origin();

void aos_main(void) {

    kprintf("aos_main\n");

//    int i=1/0; /* 除法错误正常 */

    /* 初始化进程表之前首先需要获取内核的映像信息 */
    if (get_kernel_map(&kernel_base, &kernel_limit) != OK) {
        /* 如果获取内核映像错误，那么用户进程就不能启动起来，打印错误并死机（死循环，
         * 这里键盘驱动还没有启动，不能使用painc进行错误宕机）
         */
        kprintf("get kernel map failed!!!\n");
        assert(0);
    } else {
        kprintf("kernel file begin:%d, end:%d.\n", kernel_base, kernel_base + kernel_limit);
    }
    kprintf("\n                                  A O S v1.0                                   \n");

    /**
     * 进程表的所有表项都被标志为空闲;
     * 对用于加快进程表访问的 p_proc_addr 数组进行初始化。
     */
    register Process *p_proc;
    register int logicNum;
    for (p_proc = BEG_PROC_ADDR, logicNum = -NR_TASKS; p_proc < END_PROC_ADDR; p_proc++, logicNum++) {
        if (logicNum > 0)    /* 系统服务和用户进程 */
            strcpy(p_proc->name, "unused");
        p_proc->logicIndex = logicNum;
        gp_procs[logic_nr_2_index(logicNum)] = p_proc;

    }

    /**
     * 初始化多任务支持
     * 为系统任务和系统服务设置进程表，它们的堆栈被初始化为数据空间中的数组
     */
    SysProc *p_sysProc;
    reg_t sysProcStackBase = (reg_t) sysProcStack;
    u8_t privilege;        /* CPU 权限 */
    u8_t rpl;               /* 段访问权限 */
    for (logicNum = -NR_TASKS; logicNum <= NR_LAST_TASK; logicNum++) {  /* 遍历整个系统任务 */
        p_proc = proc_addr(logicNum);                                   /* 拿到系统任务对应应该放在的进程指针 */
        p_proc->pid = logicNum;                                         /* 系统服务的pid从-NR_TASKS到-1 */
        p_sysProc = &sysProcs[logic_nr_2_index(logicNum)];              /* 系统进程项 */
        strcpy(p_proc->name, p_sysProc->name);                          /* 拷贝名称 */
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
        if (!is_idle_hardware(logicNum)) ready(p_proc);
    }

#ifndef ENABLE_TEST
    init_origin();
#endif
    /**
     * 设置消费进程，它需要一个初值。因为系统闲置刚刚启动，所以此时闲置进程是一个最合适的选择。
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

PRIVATE void init_origin() {
    Process *origin = proc_addr(ORIGIN_PROC_NR);
    init_segment_desc(&origin->ldt[CS_LDT_INDEX],
            /**
             * base最好为0，虽然会有一点浪费，但如果使用kernel_base，而内核挂载点高于0x475L，
             * 那么在初始化hd时（要读取内存0x475L处的BIOS信息），那么会触发GP异常。
             */
                      0,
                      (kernel_base + kernel_limit) >> LIMIT_4K_SHIFT, /* limit是大小-1 */
                      DA_32 | DA_LIMIT_4K | DA_C | USER_PRIVILEGE << 5
    );
    init_segment_desc(&origin->ldt[DS_LDT_INDEX],
                      0,
                      (kernel_base + kernel_limit) >> LIMIT_4K_SHIFT,
                      DA_32 | DA_LIMIT_4K | DA_DRW | USER_PRIVILEGE << 5
    );

    SegDescriptor *sdp = &origin->ldt[CS_LDT_INDEX];
    int ldt_text_limit, ldt_data_limit, ldt_data_base, ldt_data_size;
    /* 代码段基址 */
    origin->map.base = reassembly(sdp->baseHigh, 24,
                                  sdp->baseMiddle, 16,
                                  sdp->baseLow);
    /* 代码段界限，单位以段粒度计算，要么字节要么4KB */
    ldt_text_limit = reassembly(0, 0,
                                (sdp->granularity & 0xF), 16,
                                sdp->limitLow);
    /* 代码段大小 */
    origin->map.size = ((ldt_text_limit + 1) *
                        ((sdp->granularity & (DA_LIMIT_4K >> 8)) ? 4096 : 1));
    /* 然后是数据段，堆栈段共用这一块区域。 */
    sdp = &origin->ldt[DS_LDT_INDEX];
    /* 数据段&堆栈段基址 */
    ldt_data_base = reassembly(sdp->baseHigh, 24,
                               sdp->baseMiddle, 16,
                               sdp->baseLow);
    /* 数据段&堆栈段界限 */
    ldt_data_limit = reassembly((sdp->granularity & 0xF), 16,
                                0, 0,
                                sdp->limitLow);
    /* 数据段&堆栈段大小 */
    ldt_data_size = ((ldt_data_limit + 1) *
                     ((sdp->granularity & (DA_LIMIT_4K >> 8)) ? 4096 : 1));

    /**
     * 我们并不加以细分正文、数据以及堆栈段，所以TEXT和DATA段的内存映像应该是相等的，
     * 如果不一致，那么系统就不必要继续向下启动了，它有可能为以后带来隐患。
     */
    if ((origin->map.base != ldt_data_base) ||
        (ldt_text_limit != ldt_data_limit) ||
        (origin->map.size != ldt_data_size)) {
        kprintf("TEXT segment not equals DATA & STACK segment!!!\n");
        assert(0);
    }

//    reg_t procStackBase=ORIGIN_TASK_STACK_BASE;
//    origin->stackGuardWord = (reg_t *) procStackBase;
//    *origin->stackGuardWord = SYS_SERVER_STACK_GUARD;

    origin->regs.cs = ((CS_LDT_INDEX * DESCRIPTOR_SIZE) | SA_TIL | USER_PRIVILEGE); /* cs应=0 设置TI=1，表示访问的是LDT */
    origin->regs.ds = ((DS_LDT_INDEX * DESCRIPTOR_SIZE) | SA_TIL | USER_PRIVILEGE); /* ds应=8 */
    origin->regs.es = origin->regs.fs = origin->regs.ss = origin->regs.ds;  /* C 语言不加以区分这几个段寄存器 */
    origin->regs.gs = ((KERNEL_GS_SELECTOR | USER_PRIVILEGE) & SA_RPL_MASK);     /* gs 指向显存 */
    origin->regs.eip = (reg_t) origin_task;                        /* eip 指向要执行的代码首地址 */
    origin->regs.esp = (reg_t) originStack + ORIGIN_TASK_STACK;                           /* 设置栈顶 */
    origin->regs.eflags = is_task_proc(origin) ? INIT_TASK_PSW : INIT_PSW; /* 设置if位 */

    /* 进程刚刚初始化，让它处于可运行状态，所以标志位上没有1 */
    origin->flags = CLEAN_MAP;

    origin->priority = PROC_PRI_USER;
    origin->logicIndex = ORIGIN_PROC_NR;
    origin->pid = ORIGIN_PID;
    strcpy(origin->name, "origin");

    origin->level = MAX_LEVEL;
#ifdef LEVEL_SCHEDULE
    origin->wait = 0;
    origin->service = origin->level;
#endif
    ready(origin);
}