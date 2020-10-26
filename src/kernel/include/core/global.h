/**
 * 内核所需要的全局变量
 */

#ifndef AOS_GLOBAL_H
#define AOS_GLOBAL_H

#include "protect.h"

/* 全局描述符表GDT */
SegDescriptor g_gdt[];

/* 任务状态段TSS(Task-State Segment) 104字节 */
TSS g_tss; /* 用于内核任务 */

u8_t gp_gdt[6];                             /* GDT指针，0~15：Limit 16~47：Base */
u8_t gp_idt[6];                             /* IDT指针，同上 */

int g_dispPosition;                        /* low_print函数需要它标识显示位置 */

irq_handler g_irqHandlers[NR_IRQ_VECTORS];

/* 其他 */
BootParam *gp_bootParam;   /* 引导参数指针 */

#endif // AOS_GLOBAL_H
