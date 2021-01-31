//
// Created by 杜科 on 2021/1/31.
//

#include "core/kernel.h"

PUBLIC phys_page alloc(phys_page applyPages) {
    Message msg;
    msg.type = ALLOC;
    msg.PAGE_SIZE = applyPages;
    send_rec(MM_TASK, &msg);

    return msg.PAGE_ADDR != NO_MEM ? msg.PAGE_ADDR : -1;
}

/* 使用slab管理小内存的申请 */
PUBLIC u32_t malloc(u32_t size) {

    return 0;
}