//
// Created by 杜科 on 2020/10/16.
//
/**
 * 保护模式相关的一些信息
 *
 * 处理器的保护模式提供了四种特权级，0~3，0特权最大。
 * AOS使用了全部特权级，Minix使用了三种(0、1和3)，而Linux和Windows则只使用了两种（0、3）。
 */

#ifndef AOS_PROTECT_H
#define AOS_PROTECT_H

/* 描述符表指针结构 */
typedef struct descriptor_s{
    char limit[sizeof(u16_t)];
    char base[sizeof(u32_t)];		/* really u24_t + pad for 286 */
} DescriptorTablePtr;

#define DESCRIPTOR_SIZE    8        /* 描述符大小，字节为单位 */

/* 段描述符 8字节 64位 */
typedef struct seg_descriptor_s{
    u16_t limitLow;         /* 段界限低16位 */
    u16_t baseLow;          /* 段基址低16位 */
    u8_t baseMiddle;        /* 段基址中8位 */
    u8_t access;	        /* 访问权限：| P | DL | 1 | X | E | R | A | */
    u8_t granularity;       /* 比较杂，最重要的有段粒度以及段界限的高4位| G | X  | 0 | A | LIMIT HIGHT | */
    u8_t baseHigh;          /* 段基址高8位 */
} SegDescriptor;

/* 门描述符 */
typedef struct gate_s{
    u16_t	offsetLow;	    /* Offset Low */
    u16_t	selector;	    /* Selector */
    /**
     * 该字段只在调用门描述符中有效。
     * 如果在利用调用门调用子程序时引起特权级的转换和堆栈的改变，需要将外层堆栈中的参数复制到内层堆栈。
     * 该双字计数字段就是用于说明这种情况发生时，要复制的双字参数的数量。
     */
    u8_t	dcount;

    u8_t	attr;		    /* P(1) DPL(2) DT(1) TYPE(4) */
    u16_t	offsetHigh;	    /* Offset High */
} GateDescriptor;

/* 任务状态段 共104字节 */
typedef struct tss_s{
    reg_t   backLink;       /* 低16位存储一个任务的链接，高16位保留为0 */
    reg_t	esp0;	        /* 特权级0所用的堆栈 */
    reg_t	ss0;	        /* 高16位保留为0 */
    reg_t	esp1;
    reg_t	ss1;
    reg_t	esp2;
    reg_t	ss2;
    reg_t   cr3;
    reg_t	eip;
    reg_t	eflags;
    reg_t	eax;
    reg_t	ecx;
    reg_t	edx;
    reg_t	ebx;
    reg_t	esp;
    reg_t	ebp;
    reg_t	esi;
    reg_t	edi;
    reg_t	es;             /* 高16位保留为0 */
    reg_t	cs;
    reg_t	ss;
    reg_t	ds;
    reg_t   fs;
    reg_t   gs;
    reg_t	ldtSelector;    /* 高16位保留为0 */
    u16_t   trap;
    u16_t   ioMapBase;      /* I/O位图基址大于或等于TSS段界限，就表示没有I/O许可位图 */
} TSS;

/* 表大小 */
#define GDT_SIZE (LDT_FIRST_INDEX + NR_TASKS + NR_PROCS)    /* 全局描述符表 共40项*/
#define IDT_SIZE (INT_VECTOR_SYS_CALL + 1)                  /* 只取最高的向量 */
/**
 * AOS每个进程只有两个段，一个是正文段（代码段）
 * 另一个是数据段，而堆栈段则和数据段共用，以后可能会分的更细，但对于现在来说，这样就够了。
 */
#define LDT_SIZE         2

/* 固定的全局描述符索引 */
#define DUMMY_INDEX         0   /* GDT的头，标志性，不可或缺，供cpu识别 */
#define TEXT_INDEX          1   /* 0~4G，32位可读代码段 */
#define DATA_INDEX          2   /* 0~4G，32位可读写数据段 */
#define VIDEO_INDEX         3   /* 显存首地址，特权级3 */
#define TSS_INDEX           4   /* 任务状态段 */
#define LDT_FIRST_INDEX     5   /* 本地描述符 35个任务*/

/* 接下来是选择子，选择子 = (描述符索引 * 描述符大小) */
#define DUMMY_SELECTOR      DUMMY_INDEX * DESCRIPTOR_SIZE
#define TEXT_SELECTOR       TEXT_INDEX * DESCRIPTOR_SIZE                /* 低3位为0 */
#define DATA_SELECTOR       DATA_INDEX * DESCRIPTOR_SIZE
#define VIDEO_SELECTOR      VIDEO_INDEX * DESCRIPTOR_SIZE | SA_RPL3     /* DPL(特权级) = 3 */
#define SELECTOR_TSS        TSS_INDEX * DESCRIPTOR_SIZE
#define SELECTOR_LDT_FIRST  LDT_FIRST_INDEX * DESCRIPTOR_SIZE

#define KERNEL_CS_SELECTOR  TEXT_SELECTOR		/* 0~4G，32位可读代码段   */
#define KERNEL_DS_SELECTOR  DATA_SELECTOR	    /* 0~4G，32位可读写数据段 */
#define KERNEL_GS_SELECTOR  VIDEO_SELECTOR		/* 显存首地址，特权级3 */


/* 固定的局部描述符索引 */
#define CS_LDT_INDEX     0	/* 进程的代码段 */
#define DS_LDT_INDEX     1	/* 进程数据段=ES=FS=SS */


/**
 * AOS使用的四种CPU特权级
 *
 * 核心的最中心部分,即运行于中断处理期间的部分和切换进程的部分运行在INTR_PRIVILEGE特权级。
 * 在该特权级上,进程可以访问全部的内存空间和CPU的全部寄存器。
 *
 * 系统任务运行在TASK_PRIVILEGE特权级。该特权级允许它们访问I/O,但不能使用那些修改特殊寄存器(如
 * 指向描述符表的寄存器)值的指令。
 *
 * 服务器进程运行在SERVER_PRIVILEGE特权级。运行在该特权级的进程
 * 不能执行某些指令,如访问I/O端口、改变内存分配状况,或改变处理器运行级别等等。
 *
 * 用户进程运行在USER_PRIVILEGE特权级。运行在该特权级的进程限制和SERVER_PRIVILEGE
 * 一样。区别在于服务特权级可以阅读用户特权级中的代码和数据。
 */

#define KERNEL_PRIVILEGE    0	/* 内核和中断处理程序 */
#define TASK_PRIVILEGE      1
#define SERVER_PRIVILEGE    2
#define USER_PRIVILEGE      3

/* 选择子类型值说明，其中　SA_ : Selector Attribute 选择子属性 */
#define SA_RPL_MASK         0xFFFC
#define SA_RPL0             0
#define SA_RPL1             1
#define SA_RPL2             2
#define SA_RPL3             3

#define SA_TI_MASK          0xFFFB
#define SA_TIG              0
#define SA_TIL              4       /* 选择子 */

/* 描述符类型值说明，其中　DA_ : Descriptor Attribute 描述符属性 */
#define	DA_32			    0x4000	/* 32 位段			     */
#define	DA_LIMIT_4K		    0x8000	/* 段界限粒度为 4K 字节	   */
#define	LIMIT_4K_SHIFT	    12
#define	DA_DPL0			    0x00	/* DPL = 0	内核级		    */
#define	DA_DPL1			    0x20	/* DPL = 1				*/
#define	DA_DPL2			    0x40	/* DPL = 2				*/
#define	DA_DPL3			    0x60	/* DPL = 3	用户级			*/

/* 存储段描述符类型值说明 */
#define	DA_DR			    0x90	/* 存在的只读数据段类型值		*/
#define	DA_DRW			    0x92	/* 存在的可读写数据段属性值		*/
#define	DA_DRWA			    0x93	/* 存在的已访问可读写数据段类型值	*/
#define	DA_C			    0x98	/* 存在的只执行代码段属性值		*/
#define	DA_CR			    0x9A	/* 存在的可执行可读代码段属性值		*/
#define	DA_CCO			    0x9C	/* 存在的只执行一致代码段属性值		*/
#define	DA_CCOR			    0x9E	/* 存在的可执行可读一致代码段属性值	*/

/* 系统段描述符类型值说明 */
#define	DA_LDT			    0x82	/* 局部描述符表段类型值			*/
#define	DA_TaskGate		    0x85	/* 任务门类型值				*/
#define	DA_386TSS		    0x89	/* 可用 386 任务状态段类型值		*/
#define	DA_386CGate		    0x8C	/* 386 调用门类型值			*/
#define	DA_386IGate		    0x8E	/* 386 中断门类型值			*/
#define	DA_386TGate		    0x8F	/* 386 陷阱门类型值			*/

/* 宏：通过一个段描述符中的信息得到对应的物理地址，例如正文段的基地址。 */
#define	reassembly(high, high_shift, mid, mid_shift, low)   (((high) << (high_shift)) | ((mid)  << (mid_shift)) | (low))

#endif //AOS_PROTECT_H
