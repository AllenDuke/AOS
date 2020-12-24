//
// Created by 杜科 on 2020/10/15.
//
/**
 * 定义一些常用的数据类型
 * 注意：所有的类型名都以“_t”结尾，看齐是POSIX标准的规定，而且“_t”不用于非数据类型的其他任何符号名。
 * 实际上这是可以自定义的，只是一个保留习惯，或者有利于移植
 */

#ifndef AOS_TYPES_H
#define AOS_TYPES_H


/* 物理地址（字节长度） */
typedef unsigned int phys_addr;

/* 虚拟地址（字节长度） */
typedef unsigned int vir_addr;

/* 寄存器数据类型，32位无符号 */
typedef unsigned int reg_t; //todo 加register修饰

/* 物理内存页，一页4KB */
typedef unsigned int phys_page;

/* 端口数据类型，用于访问I/O端口 */
typedef unsigned port_t;

typedef int pid_t;	   /* 进程号（必须是有符号） */

/**
 * 类型size_t包含sizeof操作符的所有结果。乍一看，似乎很明显它应该是无符号整数，但情况
 * 并不总是如此。例如，有些时候(例如在68000处理器的机器上)有32位指针和16位整数。当要求
 * 70K结构或数组的大小时，结果需要表示17位，因此size_t必须是长类型。类型ssize_t是
 * size_t的有符号版本。
 */
#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
typedef int ssize_t;
#endif

#ifndef _TIME_T
#define _TIME_T
typedef unsigned long time_t;		   /* 时间自1970年1月1日格林尼治时间0000 */
#endif

#ifndef _CLOCK_T
#define _CLOCK_T
typedef unsigned long clock_t;		   /* 系统时钟计时单位 */
#endif

#ifndef _SIGSET_T
#define _SIGSET_T
typedef unsigned long sigset_t;     	/* 信号集 */
#endif



typedef unsigned char   u8_t;	    /* 8位类型 == db */
typedef unsigned short u16_t;	    /* 16位类型 == dw */
typedef unsigned int   u32_t;	    /* 32位类型 == dd */

typedef char            i8_t;       /* 8位有符号类型 */
typedef short          i16_t;       /* 16位有符号类型 */
typedef long           i32_t;       /* 32位有符号类型 */

typedef unsigned char   bool_t;     /* 布尔值 */

typedef unsigned long long u64_t;

/* 信号处理程序类型，例如SIG_IGN */
typedef void (*sighandler_t) (int);

/* 硬件（异常）中断处理函数原型 相当于声明函数指针 int_handler_t 不能如此声明 void int_handler_t (void) */
typedef void (*int_handler)(void);

/* 中断请求处理函数原型 */
typedef int (*irq_handler)(int irq);

/* 系统调用函数原型 */
typedef void (*aos_syscall)(void);

#endif //AOS_TYPES_H
