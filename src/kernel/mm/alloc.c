//
// Created by 杜科 on 2020/12/24.
//
#include "core/kernel.h"

extern CardNode nodes[];

PRIVATE void change_down(int i,bool_t val);

PRIVATE void false_up(int i);

PRIVATE int  alloc_mem(int i, phys_page applyPages);

PRIVATE int find(phys_page begin,phys_page size);

PRIVATE void check_bro_true_up(int i);

PRIVATE phys_page to_one_bit(phys_page size);

PRIVATE void init_recursively(int i,CardNode* cur,phys_page base, phys_page freePages);

PRIVATE phys_page s_totalPages;
PRIVATE phys_page s_freePages;

///* 为了不用递归，这里弄个辅助栈 */
//typedef struct entry_s{
//    CardNode *node;
//    int index;
//    phys_page begin;
//    phys_page freePages;
//}Entry;
//PRIVATE Entry stack[2]; /* 非递归版，同样需要比较大的辅助栈 */
//PRIVATE u8_t stackSize=0;

/* 从下标0开始初始化内存 */
PUBLIC void mem_init(phys_page base, phys_page freePages){
    s_totalPages=freePages;
    s_freePages=freePages;
    init_recursively(0,&nodes[0],base,freePages);
}

/**
 * 递归建造二叉堆
 * @param i 当前节点下标
 * @param cur 当前节点
 * @param base 当前节点管理的的内存的起始地址
 * @param freePages 当前节点管理的页数
 */
PRIVATE void init_recursively(int i,CardNode* cur,phys_page base, phys_page freePages) {
    cur->base = base;
    cur->len = freePages;
    cur->available = TRUE;
    if (freePages > 1) { /* 注意，因为这里用到了递归，所以应当适当增大mm的栈，而且这里会耗点时间来计算 */
        int l=2*i+1;
        int r=2*i+2;
        phys_page lf = freePages >> 1;
        phys_page rf = freePages-lf; /* 在不能除尽4k时，右边占多，实际上我假定是一样多的 */
        init_recursively(l,&nodes[l],base,lf);
        init_recursively(r,&nodes[r],base+lf,rf);
        return;
    }
}

/**
 * 从root开始寻找合适的连续页面，优先低地址
 * @param applyPages 申请页面数
 * @return 页面起始物理地址 | NO_MEM
 */
PUBLIC phys_page alloc_page(phys_page applyPages) {
//    kprintf("allocating:%d\n",applyPages);
    int i = alloc_mem(0, applyPages);
    if (i>=NR_TREE_NODE ) return NO_MEM;
//    kprintf("founded:%d\n",i);
    change_down(i,FALSE);
    false_up(i);
    s_freePages-=nodes[i].len;
    return nodes[i].base;
}

/**
 * 释放一段内存，begin和size必定属于某一节点
 * @param begin 要释放的内存的起始地址
 * @param size 要释放的内存的大小
 */
PUBLIC void free_page(phys_page begin,phys_page size){
    if(begin<0||size<0) panic("mem free begin and size must be >=0\n",PANIC_ERR_NUM);
    size=to_one_bit(size);
    int i = find(begin,size);
    change_down(i,TRUE);
    if(i==0) return;
    check_bro_true_up(i);
    s_freePages+=nodes[i].len;
}

/**
 * 检查size是否是一个2的次方数，如果是返回size，如果不是，返回size的上界2的次方数
 * 如to_one_bit(16)==16,to_one_bit(15)==16
 * @param size
 * @return size | size的上界2的次方数
 */
PRIVATE phys_page to_one_bit(phys_page size) {
    u8_t oneCount = 0; /* 按位统计size中1的数量 */
    phys_page tmp=size;
    u8_t highestOneI=0; /* size中最高位1所在位置 */
    for (u8_t i = 0; i < 32; i++) {
        if ((tmp & 1) == 1) { /* size的第i位为1 */
            oneCount++;
            highestOneI=i;
        }
        tmp =tmp >> 1;
    }
    if (oneCount == 1) return size;
    return 1<<(highestOneI+1);
}

/**
 * 从cur开始向下寻找合适的节点
 * @param cur
 * @param applyPages
 * @return 合适的节点 | NIL_CARD_NODE
 */
PRIVATE int  alloc_mem(int i, phys_page applyPages) {
    if (i>=NR_TREE_NODE || nodes[i].len < applyPages || (nodes[i].len == applyPages && nodes[i].available == FALSE)) {
        return NR_TREE_NODE;
    }
//    kprintf("cur i:%d, base:%d, len:%d\n",i,nodes[i].base,nodes[i].len);

    /* 以下为 1. root->len==applyPages&&root->available==TRUE 2. root->len>applyPages */
    if (nodes[i].len == applyPages) return i;

    /* 以下为root->len>applyPages的情况 */
    int left = alloc_mem(2*i+1, applyPages);
    if (left <NR_TREE_NODE) {
        return left;
    }

    int right = alloc_mem(2*i+2, applyPages);
    if (right <NR_TREE_NODE) {
        return right;
    }

    if(nodes[i].available==TRUE) return i;
    return NR_TREE_NODE;

}

/* 从当前节点开始，向下修改所有节点为 available=val */
PRIVATE void change_down(int i, bool_t val) {
    if(i>=NR_TREE_NODE) return;
    nodes[i].available = val;
    change_down(2*i+1, val);
    change_down(2*i+2, val);
}

/**
 * 从当前节点开始，向上更新 available=false
 * @param i
 */
PRIVATE void false_up(int i) {
    if(i<0) panic("array index err.\n",i);
    nodes[i].available=FALSE;
    if(i==0) return;
    false_up((i-1)>>1);
}

/**
 * 从当前节点开始，检查兄弟节点，向上更新 available=true
 * @param i
 */
PRIVATE void check_bro_true_up(int i) {
    if(i>=NR_TREE_NODE) return;
    nodes[i].available=TRUE;
    if(i==0) return;
    int pi=(i-1)>>1;
    if(i==2*pi+1&&nodes[2*pi+2].available==FALSE) return;
    if(i==2*pi+2&&nodes[2*pi+1].available==FALSE) return;
    check_bro_true_up(pi);
}

/**
 * begin和size必定属于某一节点，根据此信息找到该节点
 * @param begin
 * @param size
 * @return
 */
PRIVATE int find(phys_page begin,phys_page size){
    int i=0;
    while(i<NR_TREE_NODE&&(nodes[i].base!=begin||nodes[i].len!=size)){
        if (nodes[2*i+2].base <= begin) i = 2*i+2; /* cur位于node的右子树 */
        else i=2*i+1; /* cur位于node的左子树 */
    }
    if(i>=NR_TREE_NODE){
        kprintf("cloud not find node base:%d,len:%d\n",begin,size);
        panic("something wrong while alloc or free memory.\n",i);
    }
    return i;
}


PUBLIC void mem_dump(){
    kprintf("{MM}->freePages:%d\n",s_freePages);
}