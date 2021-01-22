//
// Created by 杜科 on 2021/1/18.
//

#ifndef AOS_ORIGIN_H
#define AOS_ORIGIN_H

#include "core/constant.h"

#define NR_CMD         8                    /* 预设命令数量，该值为2的次方数，利于利用位运算，加快速度 */
#define NR_CMD_TSIZE    NR_CMD*2            /*  存放cmd的HashTable的大小，冲突时线程探测，2倍是有道理的，在hash.c中有解释 */

/**
 * 用户例程指针。
 * argc是参数个数，argv的大小。其中，argv[0]是用户例程的全限定名。
 */
typedef int (*UserTask)(int argc, char *argv[]);

#define NO_USER_TASK        ((UserTask) -1)
#define NO_NODE             ((HashTableNode*) 0)

typedef struct hash_table_node_s {
    char *cmdName;
    int keyHashVal;
    UserTask userTask;                      /* 这是一个函数指针，指向实际要执行的任务 */
} HashTableNode;

void put(char *name, int cmdLen, UserTask task, HashTableNode tableNode[]);
HashTableNode * get(char *name, int cmdLen, HashTableNode tableNode[]);

int pwd(int argc, char *argv[]);
int date(int argc, char *argv[]);

#endif //AOS_ORIGIN_H
