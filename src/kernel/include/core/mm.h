//
// Created by 杜科 on 2020/12/23.
//

#ifndef AOS_MM_H
#define AOS_MM_H

/**
 * 二叉堆的形式
 * 一个节点管理一段连续的页，这里以页为单位 ，比用树的形式减少12字节的指针大小，
 * 同时借鉴了伙伴系统和Netty的内存管理机制
 */
typedef struct card_node_s { /* 可能对齐，12字节*/
    phys_page base;
    phys_page len;
    bool_t available; /* TRUE为辖下可用，可全部借出，FALSE为不可全部借出 */
} CardNode;

#define NIL_CARD_NODE (CardNode*) 0

#endif //AOS_MM_H
