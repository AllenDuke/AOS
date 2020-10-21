//
// Created by 杜科 on 2020/10/15.
//
/**
 * 一些结构体类型
 */
#ifndef AOS_STRUCT_TYPE_H
#define AOS_STRUCT_TYPE_H

#ifndef AOS_TYPES_H
#include "types.h"
#endif


/* 引导参数 */
typedef struct{
    u32_t memory_size;          /* 内存大小 */
    phys_addr kernel_file;     /* 内核所在绝对物理地址 */
} BootParam;

/* StackFrame定义了如何将寄存器值保存到堆栈上的数据结构
 * 这个结构非常重要-在进程被投入运行状态或被脱开运行状态时,它被用来保
 * 存和恢复CPU的内部状态。将其定义成可以用汇编语言高速读写的格式,这将
 * 减少进程上下文切换的时间，进程指针必须指向这里。
 */
typedef struct{
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

#endif //AOS_STRUCT_TYPE_H
