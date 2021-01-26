//
// Created by 杜科 on 2020/10/16.
//
/**
 * protect.c包含与Intel处理器保护模式相关的例程。
 */

#include "include/core/kernel.h"

/* 中断描述符表IDT */
PRIVATE GateDescriptor s_idt[IDT_SIZE];

/* 中断门信息 */
typedef struct gate_desc_s{
    u8_t vector;            /* 中断向量号 */
    int_handler handler;  /* 处理例程，这相当于一个32位的函数指针 */
    u8_t privilege;         /* 门权限 */
} GateInfo;


/* 中断门信息表 */
GateInfo s_initGateInfos[] = {
        /* 暂时只考虑i386中的0~16号异常 */
        {0x0,   divide_error,           KERNEL_PRIVILEGE},
        {0x1,   debug_exception,        KERNEL_PRIVILEGE},
        {0x2,   non_maskable_int,       KERNEL_PRIVILEGE},
        {0x3,   break_point,            KERNEL_PRIVILEGE},
        {0x4,   over_flow,              KERNEL_PRIVILEGE},
        {0x5,   out_of_bounds,          KERNEL_PRIVILEGE},
        {0x6,   invalid_opcode,         KERNEL_PRIVILEGE},
        {0x7,   dev_not_available,      KERNEL_PRIVILEGE},
        {0x8,   double_fault,           KERNEL_PRIVILEGE},
        {0x9,   coop_proc_seg_oob,      KERNEL_PRIVILEGE},
        {0xA,   invalid_tss,            KERNEL_PRIVILEGE},
        {0xB,   segment_not_present,    KERNEL_PRIVILEGE},
        {0xC,   stack_exception,        KERNEL_PRIVILEGE},
        {0xD,   general_protection,     KERNEL_PRIVILEGE},
        {0xE,   page_fault,             KERNEL_PRIVILEGE},
        /* 中断向量号 0xF 为intel保留，未使用 */
        {0x10,  math_fault,             KERNEL_PRIVILEGE},
        /* 17~19是80486后支持，20~31为intel保留，未使用 */

        /* 硬件中断 32~47 */
        { INT_VECTOR_IRQ0 + 0, hwint00, KERNEL_PRIVILEGE },
        { INT_VECTOR_IRQ0 + 1, hwint01, KERNEL_PRIVILEGE },
        { INT_VECTOR_IRQ0 + 2, hwint02, KERNEL_PRIVILEGE },
        { INT_VECTOR_IRQ0 + 3, hwint03, KERNEL_PRIVILEGE },
        { INT_VECTOR_IRQ0 + 4, hwint04, KERNEL_PRIVILEGE },
        { INT_VECTOR_IRQ0 + 5, hwint05, KERNEL_PRIVILEGE },
        { INT_VECTOR_IRQ0 + 6, hwint06, KERNEL_PRIVILEGE },
        { INT_VECTOR_IRQ0 + 7, hwint07, KERNEL_PRIVILEGE },
        { INT_VECTOR_IRQ8 + 0, hwint08, KERNEL_PRIVILEGE },
        { INT_VECTOR_IRQ8 + 1, hwint09, KERNEL_PRIVILEGE },
        { INT_VECTOR_IRQ8 + 2, hwint10, KERNEL_PRIVILEGE },
        { INT_VECTOR_IRQ8 + 3, hwint11, KERNEL_PRIVILEGE },
        { INT_VECTOR_IRQ8 + 4, hwint12, KERNEL_PRIVILEGE },
        { INT_VECTOR_IRQ8 + 5, hwint13, KERNEL_PRIVILEGE },
        { INT_VECTOR_IRQ8 + 6, hwint14, KERNEL_PRIVILEGE },
        { INT_VECTOR_IRQ8 + 7, hwint15, KERNEL_PRIVILEGE },

        /* ************* 软件中断 *************** */
        {INT_VECTOR_LEVEL0,level0_sys_call,TASK_PRIVILEGE}, /* 提供给系统任务的系统调用：提权 */
        { INT_VECTOR_PARK, park_sys_call, USER_PRIVILEGE },
        { INT_VECTOR_UNPARK, unpark_sys_call, USER_PRIVILEGE },
        { INT_VECTOR_DELAY, delay_sys_call, USER_PRIVILEGE },
        { INT_VECTOR_SYS_CALL, aos_sys_call, USER_PRIVILEGE },
};

/* 本地函数 */
FORWARD void init_gate_desc (GateInfo *gateInfo, u8_t desc_type, GateDescriptor *p);

/* 保护模式初始化 */
PUBLIC void init_protect(void) {

    /**
     * 将 LOADER 中的 GDT 拷贝到内核中新的 gdt 中。
     * src:LOADER中旧的GDT基地址
     * dest:新的GDT基地址
     * size:旧GDT的段界限 + 1
     * 这里的src和size分别强转为无符号32位指针和无符号16位指针，是因为，分别要取低地址到高地址的4字节内容和2字节的内容
     */
    phys_copy(*((u32_t *) vir2phys(&gp_gdt[2])), vir2phys(&g_gdt),
              *((u16_t *) vir2phys(&gp_gdt[0])) + 1);
    /* 算出新 GDT 的基地址和界限，设置新的 gdt_ptr */
    u16_t *p_gdt_limit = (u16_t *) vir2phys(&gp_gdt[0]);
    u32_t *p_gdt_base = (u32_t *) vir2phys(&gp_gdt[2]);
    *p_gdt_limit = GDT_SIZE * DESCRIPTOR_SIZE - 1;
    *p_gdt_base = vir2phys(&g_gdt);

    /* 算出IDT的基地址和界限，设置新的 idt_ptr */
    u16_t *p_idt_limit = (u16_t *) vir2phys(&gp_idt[0]);
    u32_t *p_idt_base = (u32_t *) vir2phys(&gp_idt[2]);
    *p_idt_limit = IDT_SIZE * sizeof(GateDescriptor) - 1;
    *p_idt_base = vir2phys(&s_idt);
    /* 初始化所有中断门描述符到 IDT中 */
    for (GateInfo *p_gate = &s_initGateInfos[0]; p_gate < &s_initGateInfos[sizeof(s_initGateInfos)/sizeof(GateInfo)]; p_gate++) {
        init_gate_desc(p_gate, DA_386IGate, &s_idt[p_gate->vector]);
    }

    /**
     * 初始化任务状态段TSS，并为处理器寄存器和其他任务切换时应保存的信息提供空间。
     * 我们只使用了某些域的信息，这些域定义了当发生中断时在何处建立新堆栈。
     * 下面init_seg_desc的调用保证它可以用GDT进行定位。
     *
     * 实际上这里和Linux2.4一样，没有使用TSS来进行任务切换，这里所有进程使用同一个tss，多核架构中，每个cpu一个tss。
     * 只用TSS的esp0和IO许可位图
     */
    memset(&g_tss, 0, sizeof(g_tss)); /* 初始化g_tss为0 */
    g_tss.ss0 = KERNEL_DS_SELECTOR;
    init_segment_desc(&g_gdt[TSS_INDEX], vir2phys(&g_tss), sizeof(g_tss) - 1, DA_386TSS);
    g_tss.ioMapBase = sizeof(g_tss);           /* 空 I/O 位图 */

    /* 为每个进程分配唯一的 LDT */
    Process *proc = BEG_PROC_ADDR;
    for(int ldtI = LDT_FIRST_INDEX; proc < END_PROC_ADDR; proc++, ldtI++) {
        memset(proc, 0, sizeof(Process)); /* clean */
        /* 每个进程的LDT指针作为一个描述符存放在GDT中 */
        init_segment_desc(&g_gdt[ldtI], vir2phys(proc->ldt), sizeof(proc->ldt) - 1, DA_LDT);
        proc->ldtSelector = ldtI * DESCRIPTOR_SIZE; /* 这里的ldtSelector是上面LDT指针在GDT中的偏移 */
    }

}

/**
 * 初始化段描述符
 * @param p_desc 段描述符指针
 * @param base 段基址
 * @param limit 段界限
 * @param attribute 段属性
 */
PUBLIC void init_segment_desc(SegDescriptor *p_desc, phys_addr base, u32_t limit, u16_t attribute) {
    p_desc->limitLow = limit & 0x0FFFF;                                        /* 段界限 1    (2 字节) */
    p_desc->baseLow = base & 0x0FFFF;                                          /* 段基址 1    (2 字节) */
    p_desc->baseMiddle = (base >> 16) & 0x0FF;                                 /* 段基址 2    (1 字节) */
    p_desc->access = attribute & 0xFF;                                          /* 属性 1 */
    p_desc->granularity = ((limit >> 16) & 0x0F) |((attribute >> 8) & 0xF0);    /* 段界限 2 + 属性 2 */
    p_desc->baseHigh = (base >> 24) & 0x0FF;                                   /* 段基址 3    (1 字节) */
}

/**
 * 利用一个中断门信息，初始化一个中断门描述符到IDT
 * @param gateInfo 中断门信息
 * @param desc_type 门描述符的类型 其实这里就是DA_386IGate，即中断门类型
 */
PRIVATE void init_gate_desc(GateInfo *gateInfo, u8_t desc_type, GateDescriptor *p_gate) {
    u32_t base_addr = (u32_t) gateInfo->handler;
    p_gate->offsetLow = base_addr & 0xFFFF;
    p_gate->selector = KERNEL_CS_SELECTOR;
    p_gate->dcount = 0;
    p_gate->attr = desc_type | (gateInfo->privilege << 5);
    p_gate->offsetHigh = (base_addr >> 16) & 0xFFFF;
}
