//
// Created by 杜科 on 2020/10/16.
//
/**
 * 该文件是内核开始C语言开发后的第一个被执行的C函数，所以它包含一个
 * cstart函数，它完成内核的一些准备工作，为跳入内核主函数做准备。
 *
 * 该文件的入口点是：
 *  - cstart:      进入内核主函数前做一些准备工作
 */
#include <core/config.h>
#include <core/global.h>
#include "stdio.h"
/**
 * 进入内核主函数前做一些初始化工作。
 */
PUBLIC void init_c(void) {
    /* 初始化显示位置 */
    g_dispPosition = (80 * 6 + 2 * 0) * 2;

    /* 建立保护机制以及中断表 */
    init_protect();

    /* 初始化硬件中断机制 */
    init_8259A();

    /* 加载引导参数 */
    u32_t *p_bootParam = (u32_t *) BOOT_PARAM_ADDR;
    /* 断言：魔数正常 */
//    assert(p_bootParam[BP_MAGIC_INDEX] == BOOT_PARAM_MAGIC);
    if (p_bootParam[BP_MAGIC_INDEX] != BOOT_PARAM_MAGIC) panic("魔数错误\n", PANIC_ERR_NUM);

    /* 魔数正常，让我们的引导参数指针指向它 */
    gp_bootParam = (BootParam *) (BOOT_PARAM_ADDR + 4);

    kprintf("init_c done\n");
}



