//
// Created by 杜科 on 2020/10/15.
//
/**
 * 一些常量，大多来自于minix
 */

#ifndef AOS_CONSTANT_H
#define AOS_CONSTANT_H

/* 增加一个这样的定义，是为了后续可能需要在某些文件中根据某些宏来确定要不要对某个变量加extern关键字 */
#define EXTERN        extern	/* used in *.h files */

#define PRIVATE       static	/* PRIVATE x limits the scope of x */
#define PUBLIC					/* PUBLIC is the opposite of PRIVATE */
#define FORWARD       static	/* some compilers require this to be 'static' */

#define TRUE               1	/* 布尔值：真 */
#define FALSE              0	/* 布尔值：假 */

#define NULL     ((void *)0)	/* 空指针 */

/* 系统任务数量 */
#define NR_TASKS    (3 + NR_CONTROLLERS)
#define NR_SERVERS  0

#define INT_VECTOR_SYS_CALL         0x94        /* AOS 系统调用向量 */

#define PANIC_ERR_NUM        0x5050	    /* 用作panic的通用错误代号 */

/* BIOS中断向量 和 保护模式下所需的中断向量 */
#define INT_VECTOR_BIOS_IRQ0        0x00
#define INT_VECTOR_BIOS_IRQ8        0x10
#define	INT_VECTOR_IRQ0				0x20    // 32
#define	INT_VECTOR_IRQ8				0x28    // 40

/* 硬件中断数量 */
#define NR_IRQ_VECTORS      16      /* 中断请求的数量 */
/* 主8259A上的 */
#define	CLOCK_IRQ		    0       /* 时钟中断请求号 */
#define	KEYBOARD_IRQ	    1       /* 键盘中断请求号 */
#define	CASCADE_IRQ		    2	    /* 第二个AT控制器的级联启用 */
#define	ETHER_IRQ		    3	    /* 默认以太网中断向量 */
#define	SECONDARY_IRQ	    3	    /* RS232 interrupt vector for port 2  */
#define	RS232_IRQ		    4	    /* RS232 interrupt vector for port 1 */
#define	XT_WINI_IRQ		    5	    /* xt风格硬盘 */
#define	FLOPPY_IRQ		    6	    /* 软盘 */
#define	PRINTER_IRQ		    7       /* 打印机 */
/* 从8259A上的 */
#define REAL_CLOCK_IRQ      8       /* 实时时钟 */
#define DIRECT_IRQ2_IRQ     9       /* 重定向IRQ2 */
#define RESERVED_10_IRQ     10      /* 保留待用 */
#define RESERVED_11_IRQ     11      /* 保留待用 */
#define MOUSE_IRQ           12      /* PS/2 鼠标 */
#define FPU_IRQ             13      /* FPU 异常 */
#define	AT_WINI_IRQ		    14	    /* at风格硬盘 */
#define RESERVED_15_IRQ     15      /* 保留待用 */

/* 8259A终端控制器端口 */
#define INT_M_CTL           0x20    /* I/O port for interrupt controller         <Master> */
#define INT_M_CTL_MASK       0x21    /* setting bits in this port disables ints   <Master> */
#define INT_S_CTL           0xA0    /* I/O port for second interrupt controller  <Slave>  */
#define INT_S_CTL_MASK       0xA1    /* setting bits in this port disables ints   <Slave>  */
/* 中断控制器的神奇数字EOI，可以用于控制中断的打开和关闭，当然，这个宏可以被类似功能的引用 */
#define EOI                 0x20    /* EOI，发送给8259A端口1，以重新启用中断 */
#define DISABLE             0       /* 用于在中断后保持当前中断关闭的代码 */
#define ENABLE              EOI	    /* 用于在中断后重新启用当前中断的代码 */



/* 功能宏 宏的参数对类型不敏感，因此你不必考虑将何种数据类型传递给宏。*/
#define MAX(a, b)   ((a) > (b) ? (a) : (b))
#define MIN(a, b)   ((a) < (b) ? (a) : (b))
/* 将内核空间中的虚拟地址转换为物理地址。其实这里的内核数据段基址还是0 */
#define	vir2phys(addr) ((phys_addr)(KERNEL_DATA_SEG_BASE + (vir_addr)(addr)))
/* 秒 转化为 毫秒 */
#define sec2ms(s) (s * 1000)
/* 滴答 转换为 毫秒 */
#define tick2ms(t)  (t * ONE_TICK_MILLISECOND)
/* 滴答 转化为 秒 */
#define tick2sec(t)   ((time_t)tick2ms(t) / 1000)
/* 字节 转换为 KB */
#define bytes2round_k(n)    ((unsigned) (((n + 512) >> 10)))

#endif //AOS_CONSTANT_H
