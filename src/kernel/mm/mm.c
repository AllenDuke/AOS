//
// Created by 杜科 on 2020/12/23.
//
#include "core/kernel.h"

PRIVATE void mm_init();

PRIVATE CardNode root;

PRIVATE Message msg;

PUBLIC void mm_task(void) {
    mm_init();
    in_outbox(&msg, &msg);
    while (TRUE) {
        rec(ANY);


    }

}

PRIVATE void mm_init() {
    phys_page totalPages = gp_bootParam->memorySize >> 2; /* memorySize的单位是KB */

    /* 得到剩余可用的空闲内存，总内存减去程序可以使用的空间即可 */
    phys_page freePages = totalPages - PROC_BASE_PAGE;

    mem_init_root(&root);

    /* 用二叉树的形式管理，从PROC_BASE_PAGE开始到结尾的内存 */
    mem_init(&root, PROC_BASE_PAGE, freePages, NIL_CARD_NODE);

    /* 打印内存信息：内存总量、核心内存的使用和空闲内存情况 */
    kprintf("You computer's total memory size = %uKB, Available = %uKB.\n\n", totalPages << 2, freePages << 2);


}
