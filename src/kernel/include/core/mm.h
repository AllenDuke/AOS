//
// Created by 杜科 on 2020/12/23.
//

#ifndef AOS_MM_H
#define AOS_MM_H


/* 用二叉树的形式管理一段连续的页，这里以页为单位 */
typedef struct card_node_s {
    phys_page base;
    phys_page len;
    bool_t available; /* TRUE为辖下可用，可全部借出，FALSE为不可全部借出 */
    struct card_node_s *pre;
    struct card_node_s *left;
    struct card_node_s *right;
} CardNode;

#define NIL_CARD_NODE (CardNode*) 0

#endif //AOS_MM_H
