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
