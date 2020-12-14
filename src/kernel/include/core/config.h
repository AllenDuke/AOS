//
// Created by 杜科 on 2020/10/15.
//
/**
 * 一些编译首选项，同时也是编译器实际上处理的第一个文件。
 * AOS暂时只支持32位有保护模式的机器。通过gcc编译器提供的32位下的宏__i386__，64位下的宏64__x86_64__，来判断。
 */

#ifndef AOS_CONFIG_H
#define AOS_CONFIG_H

/* 机器字大小（以字节为单位） */
#if __GNUC__    /* 确定是不是GNU/GCC编译器 */
#if __i386__    /* 32位机器 */
#define _WORD_SIZE	4
#endif  // __i386__
#endif  // __GNUC__

#ifndef _WORD_SIZE
#error "AOS暂时只支持32位gcc编译器和32位机器！"
#endif

/**
 * 内核代码段、数据段基地址
 * 注意：要和GDT中设置的值保持一致！
 */
#define KERNEL_TEXT_SEG_BASE    0
#define KERNEL_DATA_SEG_BASE    0

/* 进程表中的用户进程的槽数，这个配置决定了 AOS 能同时运行多少个用户进程。 */
#define NR_PROCS          32

/* 控制器任务的数量（/dev/cN设备类）。 */
#define NR_CONTROLLERS          0

/**
 * 引导参数相关信息
 * 引导参数由加载程序存储，它们应该放在内核正在运行时也不应该去覆盖的地方， 因为内核可能随时使用它们。
 */
#define BOOT_PARAM_ADDR     0x700   /* 物理地址 */
#define BOOT_PARAM_MAGIC    0x0123   /* 引导参数魔数 */
/* 参数对应的索引号 */
#define BP_MAGIC_INDEX            0
#define BP_MEMORY_SIZE_INDEX      1
#define BP_KERNEL_FILE_INDEX      2

#endif //AOS_CONFIG_H
