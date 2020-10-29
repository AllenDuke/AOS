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
_PROTOTYPE( void init_protect, (void) );

/*================================================================================================*/
/* kernel_i386lib.asm */
/*================================================================================================*/
_PROTOTYPE( void phys_copy, (phys_addr src, phys_addr dest, u32_t size) );
_PROTOTYPE( void low_print, (char* _str) );
_PROTOTYPE( void cpu_halt, (void) );
_PROTOTYPE( u8_t in_byte, (port_t port) );
_PROTOTYPE( void out_byte, (port_t port, U8_t value) );
_PROTOTYPE( u16_t in_word, (port_t port) );
_PROTOTYPE( void out_word, (port_t port, U16_t value) );
_PROTOTYPE( void interrupt_lock, (void) );
_PROTOTYPE( void interrupt_unlock, (void) );
_PROTOTYPE( int disable_irq, (int int_request) );
_PROTOTYPE( void enable_irq, (int int_request) );

/*================================================================================================*/
/* clock.c */
/*================================================================================================*/
_PROTOTYPE(void clock_task,(void));

/* 硬件（异常）中断处理函数原型 相当于声明函数指针 int_handler_t 不能如此声明 void int_handler_t (void) 有点多态的意思*/
typedef _PROTOTYPE( void (*int_handler), (void) );
/* 中断请求处理函数原型 */
typedef _PROTOTYPE( int (*irq_handler), (int irq) );
/* 系统调用函数原型 */
typedef _PROTOTYPE( void (*aos_syscall),  (void) );

/*================================================================================================*/
/* i8259.c */
/*================================================================================================*/
_PROTOTYPE( void init_8259A, (void) );
_PROTOTYPE( void put_irq_handler, (int irq, irq_handler handler) );

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

/*================================================================================================*/
/*  硬件中断处理程序。 */
/*================================================================================================*/
_PROTOTYPE( void    hwint00, (void) );
_PROTOTYPE( void	hwint01, (void) );
_PROTOTYPE( void	hwint02, (void) );
_PROTOTYPE( void	hwint03, (void) );
_PROTOTYPE( void	hwint04, (void) );
_PROTOTYPE( void	hwint05, (void) );
_PROTOTYPE( void	hwint06, (void) );
_PROTOTYPE( void	hwint07, (void) );
_PROTOTYPE( void	hwint08, (void) );
_PROTOTYPE( void	hwint09, (void) );
_PROTOTYPE( void	hwint10, (void) );
_PROTOTYPE( void	hwint11, (void) );
_PROTOTYPE( void	hwint12, (void) );
_PROTOTYPE( void	hwint13, (void) );
_PROTOTYPE( void	hwint14, (void) );
_PROTOTYPE( void	hwint15, (void) );

_PROTOTYPE( void	softIntTest, (void) );

#endif //AOS_PROTOTYPE_H
