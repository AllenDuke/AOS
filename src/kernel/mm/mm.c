//
// Created by 杜科 on 2020/12/23.
//
#include "core/kernel.h"

PRIVATE void mm_init();

/* 一棵树管理 256-16 MB内存，一页8k，共有 30720个叶子节点，树高30，全部节点数为 30720*2-1个，占用空间约为 720KB */
PUBLIC CardNode nodes[NR_TREE_NODE]; //todo 这个若是定义在alloc中会触发GP异常，具体原因暂时搞不懂

PRIVATE Message msg;

PUBLIC void mm_task(void) {
    mm_init();
//    in_outbox(&msg, &msg);
    while (TRUE) {
        rec(ANY);


    }

}

PRIVATE void mm_init() {
//    phys_page totalPages = gp_bootParam->memorySize >> 2; /* memorySize的单位是KB */

    phys_page totalPages=(256*1024)>>3; /* todo 这里就假定 实际 是256MB */

    /* 得到剩余可用的空闲内存，总内存减去程序可以使用的空间即可 */
    phys_page freePages = totalPages - PROC_BASE_PAGE;

    mem_init(PROC_BASE_PAGE, freePages);

    /* 打印内存信息：内存总量、核心内存的使用和空闲内存情况 */
    kprintf("total memory size = %dKB, available = %dKB freePages = %d.\n", totalPages << 2, freePages << 2,freePages);

}
