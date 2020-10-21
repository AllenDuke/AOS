//
// Created by 杜科 on 2020/10/16.
//
/**
 * 内核所需的所有函数原型
 * 所有那些必须在其定义所在文件外被感知的函数的原型都放在prototype.h中。
 *
 * 它使用了_PROTOTYPE技术，这样，AOS核心便既可以使用传统的C编译器(由Kernighan和Richie定义)，
 * 例如Minix初始提供的编译器；又可以使用一个现代的ANSI标准C编译器。
 *
 * 这其中的许多函数原型是与系统相关的,包括中断和异常处理例程以及用汇编语言写的一些函数。
 */

#ifndef AOS_PROTOTYPE_H
#define AOS_PROTOTYPE_H

/*================================================================================================*/
/* init_c.c */
/*================================================================================================*/
_PROTOTYPE( void init_c, (void) );

/*================================================================================================*/
/* protect.c */
/*================================================================================================*/
_PROTOTYPE( void init_segment_desc, (SegDescriptor *p_desc, phys_addr base,u32_t limit, u16_t attribute) );
_PROTOTYPE( void protect_init, (void) );

/*================================================================================================*/
/* kernel_i386lib.asm */
/*================================================================================================*/
_PROTOTYPE( void phys_copy, (phys_addr src, phys_addr dest, u32_t size) );
_PROTOTYPE( void low_print, (char* _str) );

/*================================================================================================*/
/* 异常处理入口例程 */
/*================================================================================================*/
_PROTOTYPE( void divide_error, (void) );
_PROTOTYPE( void debug_exception, (void) );
_PROTOTYPE( void non_maskable_int, (void) );
_PROTOTYPE( void break_point, (void) );
_PROTOTYPE( void over_flow, (void) );
_PROTOTYPE( void out_of_bounds, (void) );
_PROTOTYPE( void invalid_opcode, (void) );
_PROTOTYPE( void dev_not_available, (void) );
_PROTOTYPE( void double_fault, (void) );
_PROTOTYPE( void coop_proc_seg_oob, (void) );
_PROTOTYPE( void invalid_tss, (void) );
_PROTOTYPE( void segment_not_present, (void) );
_PROTOTYPE( void stack_exception, (void) );
_PROTOTYPE( void general_protection, (void) );
_PROTOTYPE( void page_fault, (void) );
_PROTOTYPE( void math_fault, (void) );

/* 硬件（异常）中断处理函数原型 int_handler_t是一个函数指针 所以要取内容 */
typedef _PROTOTYPE( void (*int_handler_t), (void) );

#endif //AOS_PROTOTYPE_H
