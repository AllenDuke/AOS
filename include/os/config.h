//
// Created by 杜科 on 2020/10/15.
//
/**
 * 一些编译首选项，同时也是编译器实际上处理的第一个文件。
 *
 * AOS暂时只支持32位有保护模式的机器。通过gcc编译器提供的32位下的宏__i386__，64位下的宏64__x86_64__，来判断。
 */

#ifndef AOS_CONFIG_H
#define AOS_CONFIG_H

/* AOS发行版和版本号。 */
#define OS_RELEASE "0"
#define OS_VERSION "0.1"

/*===========================================================================*
 *		本节包含用户可设置的参数		     *
 *===========================================================================*/
#define MACHINE        		IBM_PC	/* Must be one of the names listed below */

#define IBM_PC				1		/* any  8088 or 80x86-based system */
#define SUN_4             	40		/* any Sun SPARC-based system */
#define SUN_4_60	  		40		/* Sun-4/60 (aka SparcStation 1 or Campus) */
#define ATARI             	60		/* ATARI本节包含用户可设置的参数 ST/STe/TT (68000/68030) */
#define AMIGA             	61		/* Commodore Amiga (68000) */
#define MACINTOSH         	62		/* Apple Macintosh (68000) */

/* 机器字大小（以字节为单位），等于sizeof(int）的常量 */
#if __ACK__     /* 确定是不是Amsterdam Comiler Kit (ACK)编译器 */
#define _WORD_SIZE	_EM_WSIZE
#endif  //__ACK__

#if __GNUC__    /* 确定是不是GNU/GCC编译器 */
#if __i386__    /* 32位机器 */
#define _WORD_SIZE	4
#elif __x86_64__
#define _WORD_SIZE	8   /* 64位机器 */
#endif  // __i386__
#endif  // __GNUC__

/* 如果在不在32位机器上编译，报错 */
#if _WORD_SIZE != 4
#error AOS暂时只支持32位编译器和32位机器！
#endif

/* 内核代码段、数据段基地址
 * 注意：要和GDT中设置的值保持一致！
 */
#define KERNEL_TEXT_SEG_BASE    0
#define KERNEL_DATA_SEG_BASE    0

/* 引导参数相关信息
 * 引导参数由加载程序存储，它们应该放在内核正在运行时也不应该去覆盖的地方，
 * 因为内核可能随时使用它们。
 */
#define BOOT_PARAM_ADDR     0x700   /* 物理地址 */
#define BOOT_PARAM_MAGIC    0x328   /* 引导参数魔数 */
#define BP_MAGIC            0
#define BP_MEMORY_SIZE      1
#define BP_KERNEL_FILE      2

/* 进程表中的用户进程的槽数，这个配置决定了AOS能同时运行多少个用户进程。 */
#define NR_PROCS          32

/* 控制器任务的数量（/dev/cN设备类）。 */
#define NR_CONTROLLERS          0

/* 包括或排除设备驱动程序。 设置为1表示包含，设置为0表示排除。 */
#define ENABLE_AT_WINI      1   /* AT风格的硬盘驱动程序 */
#define ENABLE_ATAPI        0	/* 将ATAPI支持添加到AT驱动程序 */
#define ENABLE_BIOS_WINI    0	/* BIOS硬盘驱动程序 */
#define ENABLE_ESDI_WINI    0	/* ESDI风格的硬盘驱动程序 */
#define ENABLE_XT_WINI      0	/* XT风格的硬盘驱动程序 */
#define ENABLE_AHA1540_SCSI 0	/* Adaptec 1540 SCSI 驱动程序 */
#define ENABLE_FATFILE      0	/* FAT文件虚拟磁盘驱动程序 */
#define ENABLE_DOSFILE      0	/* DOS文件虚拟磁盘驱动程序 */
#define ENABLE_SB16         0	/* Soundblaster音频驱动程序 */
#define ENABLE_PRINTER      0	/* 打印机驱动程序 */
#define ENABLE_USER_BIOS    0	/* 用户模式BIOS调用驱动 */

/* 启用或禁用网络驱动程序，默认关闭 */
#define 	ENABLE_DP8390 	0   /* DP8390以太网驱动程序 */
#define   	ENABLE_WESTERN	1	/* 将Western Digital WD80x3添加到DP8390 */
#define   	ENABLE_NE2000	1	/* 将Novell NE1000 / NE2000添加到DP8390 */
#define   	ENABLE_3C503 	1	/* 将3Com Etherlink II（3C503）添加到DP8390 */

/* 内核配置参数 */
#define LINE_WARP               1   /* 控制台选项 - 是否需要在第80列换行？ */

/* AOS所启用的控制台的数量等定义 */
#define NR_CONSOLES           	3	/* 系统控制台数量(1 ~ 9) */
#define	NR_RS_LINES	   		    0	/* rs232终端数量(0 ~ 2) */
#define	NR_PTYS		  	 	    0	/* 伪终端数量(0 ~ 64) */

/*===========================================================================*
 *	在这一行之后没有用户可设置的参数		     *
 *===========================================================================*/
/* Set the CHIP type based on the machine selected. The symbol CHIP is actually
 * indicative of more than just the CPU.  For example, machines for which
 * CHIP == INTEL are expected to have 8259A interrrupt controllers and the
 * other properties of IBM PC/XT/AT/386 types machines in general. */
#define INTEL               1	/* CHIP type for PC, XT, AT, 386 and clones */
#define M68000              2	/* CHIP type for Atari, Amiga, Macintosh    */
#define SPARC               3	/* CHIP type for SUN-4 (e.g. SPARCstation)  */

/* Set the FP_FORMAT type based on the machine selected, either hw or sw    */
#define FP_NONE		        0	/* no floating point support                */
#define FP_IEEE		        1	/* conform IEEE floating point standard     */

#if (MACHINE == IBM_PC)
#define CHIP          INTEL
#endif

#ifndef FP_FORMAT
#define FP_FORMAT   FP_NONE
#endif

#ifndef MACHINE
error "编译前请在<os/config.h>配置文件中定义你要编译的机器的宏(MACHINE)"
#endif

#ifndef CHIP
error "编译前请在<os/config.h>配置文件中定义你要编译的机器的宏(MACHINE)"
#endif

#if (MACHINE == 0)
error "MACHINE的值不正确(0)"
#endif

#endif //AOS_CONFIG_H
