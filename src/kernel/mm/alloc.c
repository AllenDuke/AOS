//
// Created by 杜科 on 2020/12/24.
//
#include "core/kernel.h"

PRIVATE CardNode *root;

PRIVATE void change_down(CardNode *cur,bool_t val);

PRIVATE void false_up(CardNode *cur);

PRIVATE CardNode *alloc_mem(CardNode *cur, phys_page applyPages);

PRIVATE CardNode* find(phys_page begin,phys_page size);

PRIVATE void check_bro_true_up(CardNode *cur);

PUBLIC void mem_init_root(CardNode *node) {
    root = node;
}

/* 伙伴系统，而minix采用动态分区，首次适配算法 */
PUBLIC void mem_init(CardNode *cur, phys_page base, phys_page freePages,CardNode* p) {
    cur->base = base;
    cur->len = freePages;
    cur->available = TRUE;
    cur->pre=p;
    if (freePages > 1) { /* 注意，因为这里用到了递归，所以应当适当增大mm的栈，而且这里会耗点时间来计算，todo 不用递归 */
        CardNode l, r;
        cur->left=&l;
        cur->right=&r;
        phys_page lf = freePages >> 1;
//        kprintf("cur:%d, left:%d, right:%d\n",freePages,lf,freePages-lf);
        mem_init(&l, base, lf,cur);
        mem_init(&r, base + lf, freePages - lf,cur); /* 在不能除尽4k时，右边占多 */
        return;
    }
    cur->left = NIL_CARD_NODE;
    cur->right = NIL_CARD_NODE;
}

/**
 * 从root开始寻找合适的连续页面，优先低地址
 * @param applyPages 申请页面数
 * @return 页面起始物理地址 | NO_MEM
 */
PUBLIC phys_page alloc(phys_page applyPages) {
    CardNode *cur = alloc_mem(root, applyPages);
    if (cur == NIL_CARD_NODE) return NO_MEM;
    change_down(cur,FALSE);
    false_up(cur);
    return cur->base;
}

/**
 * 释放一段内存，begin和size必定属于某一节点
 * @param begin 要释放的内存的起始地址
 * @param size 要释放的内存的大小
 */
PUBLIC void free(phys_page begin,phys_page size){
    CardNode *node = find(begin,size);
    change_down(node,TRUE);
    if(node==root) return;
    check_bro_true_up(node);
}

/**
 * 从cur开始向下寻找合适的节点
 * @param cur
 * @param applyPages
 * @return 合适的节点 | NIL_CARD_NODE
 */
PRIVATE CardNode *alloc_mem(CardNode *cur, phys_page applyPages) {
    if (cur == NIL_CARD_NODE || cur->len < applyPages || (cur->len == applyPages && cur->available == FALSE))
        return NIL_CARD_NODE;

    /* 以下为 1. root->len==applyPages&&root->available==TRUE 2. root->len>applyPages */
    if (cur->len == applyPages) return cur;

    /* 以下为root->len>applyPages的情况 */
    CardNode *left = alloc_mem(cur->left, applyPages);
    if (left != NIL_CARD_NODE) return left;
    CardNode *right = alloc_mem(cur->left, applyPages);
    if (right != NIL_CARD_NODE) return right;

    if(cur->available==TRUE) return cur;
    return NIL_CARD_NODE;

}

/* 从当前节点开始，向下修改所有节点为 available=val */
PRIVATE void change_down(CardNode *cur, bool_t val) {
    if (cur == NIL_CARD_NODE) return;
    cur->available = val;
    change_down(cur->left, val);
    change_down(cur->right, val);
}

/**
 * 从当前节点开始，向上更新 available=false
 * @param cur
 */
PRIVATE void false_up(CardNode *cur) {
    if(cur==NIL_CARD_NODE) return;
    cur->available=FALSE;
    false_up(cur->pre);
}

/**
 * 从当前节点开始，检查兄弟节点，向上更新 available=true
 * @param cur
 */
PRIVATE void check_bro_true_up(CardNode *cur) {
    if(cur==NIL_CARD_NODE) return;
    cur->available=TRUE;
    if(cur==root) return;
    CardNode *p=cur->pre;
    if(cur==p->left&&p->right->available==FALSE) return;
    if(cur==p->right&&p->left->available==FALSE) return;
    check_bro_true_up(cur->pre);
}

/**
 * begin和size必定属于某一节点，根据此信息找到该节点
 * @param begin
 * @param size
 * @return
 */
PRIVATE CardNode* find(phys_page begin,phys_page size){
    CardNode *node=root;
    while(node!=NIL_CARD_NODE&&(node->base!=begin||node->len!=size)){
        if (node->right->base <= begin) node = node->right; /* cur位于node的右子树 */
        else node = node->left; /* cur位于node的左子树 */
    }
    if(node==NIL_CARD_NODE){
        kprintf("找不到节点 base:%d,len:%d\n",begin,size);
        panic("申请或释放内存时发生异常\n",PANIC_ERR_NUM);
    }
    return node;
}
