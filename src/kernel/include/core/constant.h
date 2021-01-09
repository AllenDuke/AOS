//
// Created by 杜科 on 2020/10/15.
//
/**
 * 一些常量，大多来自于minix
 */

#ifndef AOS_CONSTANT_H
#define AOS_CONSTANT_H

//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  通用常量
//----------------------------------------------------------------------------------------------------------------------

#define PRIVATE         static            /* PRIVATE x limits the scope of x */
#define PUBLIC                            /* PUBLIC is the opposite of PRIVATE */
#define FORWARD         static            /* some compilers require this to be 'static' */

#define TRUE            1                /* 布尔值：真 */
#define FALSE           0                /* 布尔值：假 */
#define NULL            (void *)0       /* 空指针 */
#define NIL_PTR   (char *) 0            /* 一般且有用的表达，空指针 */

#define INIT_PSW      0x202        /* initial psw :IF=1, 位2一直是1 */
#define INIT_TASK_PSW 0x1202    /* initial psw for tasks (with IOPL 1) : IF=1, IOPL=1, 位2一直是1*/

#define NR_CMDS         8     /* 预设命令数量，该值为2的次方数，利于利用位运算，加快速度 */

//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  保护模式相关
//----------------------------------------------------------------------------------------------------------------------

#define INT_VECTOR_LEVEL0           0x30
#define INT_VECTOR_PARK             0x31
#define INT_VECTOR_UNPARK           0x32
#define INT_VECTOR_SYS_CALL         0x94        /* AOS 系统调用向量 */
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  panic相关
//----------------------------------------------------------------------------------------------------------------------
#define PANIC_ERR_NUM        0x5050        /* 用作panic的通用错误代号 */
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  硬件中断相关
//----------------------------------------------------------------------------------------------------------------------
/* BIOS中断向量 和 保护模式下所需的中断向量 */
#define INT_VECTOR_BIOS_IRQ0        0x00
#define INT_VECTOR_BIOS_IRQ8        0x10
#define    INT_VECTOR_IRQ0                0x20    // 32
#define    INT_VECTOR_IRQ8                0x28    // 40

/* 硬件中断数量 */
#define NR_IRQ_VECTORS      16      /* 中断请求的数量 */
/* 主8259A上的 */
#define    CLOCK_IRQ            0       /* 时钟中断请求号 */
#define    KEYBOARD_IRQ        1       /* 键盘中断请求号 */
#define    CASCADE_IRQ            2        /* 第二个AT控制器的级联启用 */
#define    ETHER_IRQ            3        /* 默认以太网中断向量 */
#define    SECONDARY_IRQ        3        /* RS232 interrupt vector for port 2  */
#define    RS232_IRQ            4        /* RS232 interrupt vector for port 1 */
#define    XT_WINI_IRQ            5        /* xt风格硬盘 */
#define    FLOPPY_IRQ            6        /* 软盘 */
#define    PRINTER_IRQ            7       /* 打印机 */
/* 从8259A上的 */
#define REAL_CLOCK_IRQ      8       /* 实时时钟 */
#define DIRECT_IRQ2_IRQ     9       /* 重定向IRQ2 */
#define RESERVED_10_IRQ     10      /* 保留待用 */
#define RESERVED_11_IRQ     11      /* 保留待用 */
#define MOUSE_IRQ           12      /* PS/2 鼠标 */
#define FPU_IRQ             13      /* FPU 异常 */
#define    AT_WINI_IRQ            14        /* at风格硬盘 */
#define RESERVED_15_IRQ     15      /* 保留待用 */

/* 8259A终端控制器端口 */
#define INT_M_CTL           0x20    /* I/O port for interrupt controller         <Master> */
#define INT_M_CTL_MASK      0x21    /* setting bits in this port disables ints   <Master> */
#define INT_S_CTL           0xA0    /* I/O port for second interrupt controller  <Slave>  */
#define INT_S_CTL_MASK      0xA1    /* setting bits in this port disables ints   <Slave>  */
/* 中断控制器的神奇数字EOI，可以用于控制中断的打开和关闭，当然，这个宏可以被类似功能的引用 */
#define EOI                 0x20    /* EOI，发送给8259A端口1，以重新启用中断 */
#define DISABLE             0       /* 用于在中断后保持当前中断关闭的代码 */
#define ENABLE              EOI        /* 用于在中断后重新启用当前中断的代码 */

//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  时钟中断相关
//----------------------------------------------------------------------------------------------------------------------
/* 时钟, 8253 / 8254 PIT (可编程间隔定时器)参数 */
#define HZ                100        /* 时钟频率，即时钟一秒可以发出几次中断 */

#define TIMER0          0x40    /* 定时器通道0的I/O端口 */
#define TIMER1          0x41    /* 定时器通道1的I/O端口 */
#define TIMER2          0x42    /* 定时器通道2的I/O端口 */
#define TIMER_MODE      0x43    /* 用于定时器模式控制的I/O端口 */

#define RATE_GENERATOR            0x34                    /* 00-11-010-0 Counter0 - LSB the MSB - rate generator - binary */
#define TIMER_FREQ                1193182L                /* clock frequency for timer in PC and AT */
#define TIMER_COUNT             (TIMER_FREQ / HZ)       /* initial value for counter*/

#define CLOCK_ACK_BIT            0x80                    /* PS/2 clock interrupt acknowledge bit */

#define ONE_TICK_MILLISECOND    (1000 / HZ)             /* 一次滴答（中断）有多少毫秒，这个值由时钟频率决定 */

/* 用户进程使用时间片轮转算法，这里可以对轮转时间进行配置 */
#define SCHEDULE_MILLISECOND    100                     /* 用户进程调度的频率（毫秒），根据喜好设置就行 */
#define SCHEDULE_TICKS          (SCHEDULE_MILLISECOND / ONE_TICK_MILLISECOND)  /* 用户进程调度的频率（滴答） */

#define MINUTES                 60                    /* 1 分钟的秒数。 */
#define HOURS                   (60 * MINUTES)        /* 1 小时的秒数。 */
#define DAYS                    (24 * HOURS)        /* 1 天的秒数。 */
#define YEARS                   (365 * DAYS)        /* 1 年的秒数。 */

/* 时钟任务消息中使用的消息字段 */
#define CLOCK_TIME              m6_l1    /* 时间值 */

//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  进程相关
//----------------------------------------------------------------------------------------------------------------------
/* 与消息类型有关的定义。 */
#define M1                 1        /* 消息类型1：消息域使用mess_union1 */
#define M3                 3        /* 同上 */
#define M4                 4        /* 同上 */
#define M3_STRING         15        /* 消息类型3携带字符串的长度 */

/* 消息常用宏定义 */
#define MESSAGE_SIZE    (sizeof(Message))   /* 一个消息的字节大小 */
#define NIL_MESSAGE     ((Message *) 0)     /* 空消息 */

/* 下面定义了进程调度的三个优先级队列 */
#define TASK_QUEUE         0        /* 就绪的系统任务通过队列0调度 */
#define SERVER_QUEUE       1        /* 就绪的系统服务通过队列1调度 */
#define USER_QUEUE         2        /* 就绪的系统服务通过队列2调度 */
#define NR_PROC_QUEUE      3        /* 调度队列的数量 */

/* 系统调用例程可以支持的操作 */
#define SEND                0x1        /* 0001: 发送一条消息 */
#define RECEIVE             0x2        /* 0010: 接收一条消息 */
#define SEND_REC            0x3        /* 0011: 发送一条消息并等待对方响应一条消息 */
#define IN_OUTBOX           0x4    /* 0100: 设置固定收发件箱  */
/* 魔数，它是一个不存在的进程逻辑编号，用于表示任何进程receive(ANY, msg_buf) 表示接收任何进程的消息 */
#define ANY                 0x3ea

/* 系统任务数量 */
#ifdef ENABLE_TEST
#define NR_TASKS    (7 + NR_CONTROLLERS+1)
#else
#define NR_TASKS    (7 + NR_CONTROLLERS)
#endif
#define NR_LAST_TASK        -1       /* 系统任务的逻辑号从 NR_TASKS 到 NR_LAST_TASK */

/* 每个系统任务的任务号和它的功能服务号(消息类型)以及回复代码，将在下面开始定义 */
#define INVALID_DRIVER        -20
#ifdef ENABLE_TEST
#define TEST_TASK           -8
#endif
#define TTY_TASK            -7
#define HD_TASK             -6
#define FS_TASK             -5
#define CLOCK_TASK          -4      /* 时钟任务 */
#define MM_TASK             -3
#define IDLE_TASK           -2      /* 待机任务 */
#define HARDWARE            -1      /* 代表硬件，用于生成软件生成硬件中断，并不存在实际的任务 */

/* 一些重要进程的进程逻辑号，用于MM */
#define ORIGIN_PROC_NR       0        /* 初始化 -- 将会fork为多用户进程 */

#define ORIGIN_PID           0      /* origin 第一个用户进程，linux中init的进程号为1 */
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  键盘相关
//----------------------------------------------------------------------------------------------------------------------
/* 8042 ports */
/** I/O port for keyboard data
 * Read : Read Output Buffer
 * Write: Write Input Buffer(8042 Data&8048 Command)
 */
#define    KB_DATA        0x60
/**
 * I/O port for keyboard command
 * Read : Read Status Register
 * Write: Write Input Buffer(8042 Command)
 */
#define    KB_CMD        0x64
#define    LED_CODE    0xED
#define    KB_ACK        0xFA
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  VGA相关
//----------------------------------------------------------------------------------------------------------------------
#define CRTC_ADDR_REG                0x3D4    /* CRT Controller Registers - Address Register */
#define CRTC_DATA_REG                0x3D5    /* CRT Controller Registers - Data Registers */
#define CRTC_DATA_IDX_START_ADDR_H   0xC        /* register index of video mem start address (MSB) */
#define CRTC_DATA_IDX_START_ADDR_L    0xD        /* register index of video mem start address (LSB) */
#define CRTC_DATA_IDX_CURSOR_H        0xE        /* register index of cursor position (MSB) */
#define CRTC_DATA_IDX_CURSOR_L        0xF        /* register index of cursor position (LSB) */
#define V_MEM_BASE                    0xB8000    /* base of color video memory */
#define V_MEM_SIZE                    0x8000    /* 32K: B8000H -> BFFFFH */
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  TTY相关
//----------------------------------------------------------------------------------------------------------------------
#define TTY_FIRST    (ttys)
#define TTY_END        (ttys + NR_CONSOLES)
#define NR_CONSOLES 3
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  mm，内存管理相关
//----------------------------------------------------------------------------------------------------------------------
#define PAGE_SIZE      8*1024            /* 分配内存的单位 8K */
#define PAGE_SHIFT     13                /* 内存块位数，用于移位 */
#define TREE_SIZE      256*1024*1024    /* 一个树大小256MB 从0到255 这里暂时定死  */
#define NR_TREE_NODE   32*1024*2-1        /* 一颗树的节点数 */

/* 所有派生（fork）的进程将使用PROC_BASE之上的内存
 *
 * 注意：请确保PROC_BASE的值高于任何缓冲区，例如文件系统缓冲区，
 * 内存管理器缓冲区等等。
 * 现在它们缓冲区的长度为：0xB00000(16MB)
 */
#define FREE_BASE                 0x1000000            /* 可以安全使用的内存空间物理地址：16M */
#define PROC_BASE_PAGE            (FREE_BASE >> PAGE_SHIFT)


/* 由alloc()函数返回，用于告诉调用者，内存不足，无法完成分配。 */
#define NO_MEM  ((phys_page) 0)
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  FS相关
//----------------------------------------------------------------------------------------------------------------------
#define    NR_FILES        64
#define    NR_FILE_DESC    64    /* FIXME */
#define    NR_INODE        64    /* FIXME */
#define    NR_SUPER_BLOCK    8

/* inode索引号 */
#define    INVALID_INODE    0
#define    ROOT_INODE        1

#define    NR_DEFAULT_FILE_SECTS    2048 /* 2048 * 512 = 1MB，用于根目录 */

/**
 * 保留一些扇区供我们（操作系统创造者）在此处复制tar文件，该文件将由OS提取并使用。
 * 文件里存放一些使用的程序。
 * 注意，INSTALL_NR_SECTS应该是NR_DEFAULT_FILE_SECTS的倍数：
 *      INSTALL_NR_SECTS = n * NR_DEFAULT_FILE_SECTS（int n）
 */
#define    INSTALL_START_SECT        0x17000
#define    INSTALL_NR_SECTS        0x800

/*
 * disk log
 */
#define ENABLE_DISK_LOG
#define SET_LOG_SECT_SMAP_AT_STARTUP
#define MEMSET_LOG_SECTS
#define NR_SECTS_FOR_LOG        NR_DEFAULT_FILE_SECTS

/* INODE::i_mode (octal, lower 12 bits reserved) */
#define I_TYPE_MASK     0170000    /* 该字段给出inode类型 */
#define I_REGULAR       0100000    /* 常规文件，非目录或特殊文件 */
#define I_BLOCK_SPECIAL 0060000    /* 块特殊文件 */
#define I_DIRECTORY     0040000    /* 文件是一个目录 */
#define I_CHAR_SPECIAL  0020000    /* 字符特殊设备 */
#define I_NAMED_PIPE    0010000 /* 命名管道（FIFO） */
#define RWX_MODES       0000777    /* RWX模式位 */
#define R_BIT           0000004    /* Rwx保护位 */
#define W_BIT           0000002    /* rWx保护位 */
#define X_BIT           0000001    /* rwX保护位 */
#define I_NOT_ALLOC     0000000    /* 这个索引节点是空闲的 */
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  堆栈相关
// 堆栈过大会出现 UP异常？难道c代码被优化了？
//----------------------------------------------------------------------------------------------------------------------
/* 一个 512 字节 的小栈 */
#define SMALL_STACK         (128 * sizeof(char*))
/* 这是一个普通堆栈大小，1KB */
#define NORMAL_STACK        (256 * sizeof(char*))

#define	PROC_IMAGE_SIZE_DEFAULT	0x100000 /*  1 MB */
#define PROC_ORIGIN_STACK   0x400 /* 1kB 干嘛用的 */

#ifdef ENABLE_TEST
#define TEST_TASK_STACK     SMALL_STACK
#else
#define TEST_TASK_STACK     0
#endif
#define FS_TASK_STACK       NORMAL_STACK
#define HD_TASK_STACK       NORMAL_STACK*64
#define MM_TASK_STACK       NORMAL_STACK
/* 终端任务 */
#define TTY_TASK_STACK      (32 * NORMAL_STACK)
/* 时钟任务栈 */
#define CLOCK_TASK_STACK    SMALL_STACK
/* 待机任务堆栈 */
#define IDLE_TASK_STACK     SMALL_STACK
/* 虚拟硬件栈 */
#define HARDWARE_STACK  0
/* 所有系统进程的栈空间总大小 */
#define TOTAL_TASK_STACK    (HARDWARE_STACK+IDLE_TASK_STACK+CLOCK_TASK_STACK+TTY_TASK_STACK+MM_TASK_STACK \
                            +HD_TASK_STACK+FS_TASK_STACK+TEST_TASK_STACK)
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  功能宏，宏的参数对类型不敏感，因此你不必考虑将何种数据类型传递给宏
//----------------------------------------------------------------------------------------------------------------------
#define MAX(a, b)   ((a) > (b) ? (a) : (b))
#define MIN(a, b)   ((a) < (b) ? (a) : (b))

/* 将内核空间中的虚拟地址转换为物理地址。其实这里的内核数据段基址还是0 */
#define    vir2phys(addr)      ((phys_addr)(KERNEL_DATA_SEG_BASE + (vir_addr)(addr)))

#define sec2ms(s)           (s * 1000)                          /* 秒 转化为 毫秒 */
#define tick2ms(t)          (t * ONE_TICK_MILLISECOND)          /* 滴答 转换为 毫秒 */
#define tick2sec(t)         ((time_t)tick2ms(t) / 1000)          /* 滴答 转化为 秒 */

/* 滴答 转换为 毫秒 */
#define tick2ms(t)          (t * ONE_TICK_MILLISECOND)
/* 滴答 转化为 秒 */
#define tick2sec(t)         ((time_t)tick2ms(t) / 1000)
/* 字节 转换为 KB */
#define bytes2round_k(n)    ((unsigned) (((n + 512) >> 10)))

/* 为了消息通信调用的简洁 */
#define sen(n)              send(n, NIL_MESSAGE)
#define rec(n)              receive(n, NIL_MESSAGE)
#define sen_rec(n)          send_rec(n, NIL_MESSAGE)
#define io_box(vir)         in_outbox(vir, vir);
//======================================================================================================================


#endif //AOS_CONSTANT_H
