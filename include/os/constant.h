//
// Created by 杜科 on 2020/10/15.
//
/**
 * 一些常量，大多来自于minix
 */

#ifndef AOS_CONSTANT_H
#define AOS_CONSTANT_H

#ifndef CHIP
#error CHIP is not defined
#endif

/* 增加一个这样的定义，是为了后续可能需要在某些文件中根据某些宏来确定要不要对某个变量加extern关键字 */
#define EXTERN        extern	/* used in *.h files */

#define PRIVATE       static	/* PRIVATE x limits the scope of x */

#define PUBLIC					/* PUBLIC is the opposite of PRIVATE */

#define FORWARD       static	/* some compilers require this to be 'static' */

#define TRUE               1	/* 布尔值：真 */
#define FALSE              0	/* 布尔值：假 */

#define HZ	          	100		/* 时钟频率，即时钟一秒可以发出几次中断 */
#define ONE_TICK_MILLISECOND (1000 / HZ)  /* 一次滴答（中断）有多少毫秒，这个值由时钟频率决定 */
#define BLOCK_SIZE      1024	/* 磁盘块中的字节量 */
#define SUPER_USER      0	    /* 表示超级用户！ */

#define NULL     ((void *)0)	/* 空指针 */
#define NR_IO_REQUESTS  MIN(NR_BUFS, 64)        /* 一次io请求最多传输数量 */

#define NR_SEGS             2   /* 每个进程的拥有的段数量 */
#define TEXT                0   /* 正文段索引号，也称为代码段 */
#define DATA                1   /* 数据段索引号，堆栈段共用这个空间 */

/* 一些重要进程的进程号 */
#define MM_PROC_NR         0		/* 内存管理器 */
#define FS_PROC_NR         1		/* 文件系统 */
#define FLY_PROC_NR        2    	/* FLY */
#define ORIGIN_PROC_NR	   3		/* 初始化 -- 将会fork为多用户进程 */
#define LOW_USER        -1  /* 第一个用户进程不是操作系统的一部分 */

/* 其他 */
#define BYTE            0xFF	/* 8位字节的掩码 */
#define READING            0	/* 复制数据给用户 */
#define WRITING            1	/* 从用户那复制数据 */
#define NO_NUM        0x8000	/* 用作panic()的数值参数 */
#define NIL_PTR   (char *) 0	/* 一般且有用的表达，空指针 */
#define HAVE_SCATTERED_IO  1	/* 分散的I/O现在是标准配置 */

/* 功能宏 */
/* 取两数最大最小值，用简单的宏来实现 */
#define MAX(a, b)   ((a) > (b) ? (a) : (b))
#define MIN(a, b)   ((a) < (b) ? (a) : (b))

/* 系统任务数量 */
#define NR_TASKS    (3 + NR_CONTROLLERS)
#define NR_SERVERS  0

/* 内存是通过块分配的。 */
#if (CHIP == INTEL)
#define CLICK_SIZE      1024	/* 分配内存的单位 */
#define CLICK_SHIFT       10	/* 内存块位数，用于移位 */
#endif

#define ABSOLUTE             -0x3EA	    /* 这个进程意味着其是一个系统任务，处于物理地址 */

/* 索引节点中i_mode的标志位。 */
#define I_TYPE          0170000	/* 该字段给出inode类型 */
#define I_REGULAR       0100000	/* 常规文件，非目录或特殊文件 */
#define I_BLOCK_SPECIAL 0060000	/* 块特殊文件 */
#define I_DIRECTORY     0040000	/* 文件是一个目录 */
#define I_CHAR_SPECIAL  0020000	/* 字符特殊设备 */
#define I_NAMED_PIPE	0010000 /* 命名管道（FIFO） */
#define RWX_MODES       0000777	/* RWX模式位 */
#define R_BIT           0000004	/* Rwx保护位 */
#define W_BIT           0000002	/* rWx保护位 */
#define X_BIT           0000001	/* rwX保护位 */
#define I_NOT_ALLOC     0000000	/* 这个索引节点是空闲的 */

/* 一些限制 */
#define MAX_BLOCK_NR  ((block_t) 077777777)	/* 最大块数 */
#define MAX_INODE_NR      ((ino_t) 0177777)	/* 最大索引节点数 */
#define MAX_FILE_POS ((off_t) 037777777777)	/* 最大合法文件偏移量 */

#define NO_BLOCK              ((block_t) 0)	/* 缺少块号 */
#define NO_ENTRY                ((ino_t) 0)	/* 缺少目录项 */
#define NO_DEV                  ((dev_t) 0)	/* 缺少设备号 */


#endif //AOS_CONSTANT_H
