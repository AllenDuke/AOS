//
// Created by 杜科 on 2021/1/18.
//
#include "origin.h"

PUBLIC void putTask(char *name, int cmdLen, UserTask task, HashTableNode hashTable[]) {
    if (cmdLen <= 0) return;
    int hash = 0;
    /**
     * 类似java String hash值计算，不直接采取31是因为cmd一般比较短，而31又比较小，
     * 127这个质数比较大，可尽可能地使得hash的高位有值
     * hash=hash*127+cmd[i]
     */
    for (int i = 0; i < cmdLen; i++) {
        hash = ((hash << 8) - hash) + name[i];
    }

    /**
     * 扰动函数
     * 同java HashMap hash()
     * hash值的高16位为原hash的高16位，低16位为原hash高16位与原hash低16位异或后的结果。
     * 原因：因为数组的长度为2的次方数，所以最终计算下标时，使用(NR_CMD-1)&hash，
     * 这样一来，因为(NR_CMD_TSIZE-1)的高位总是0，hash的高位没有参与到运算，为了进一步降低冲突，应使得高位也参与运算
     * 在权衡质量与速度后，选择了这种方式。成本不高，质量也不错。
     */
    hash = hash ^ (hash >> 16);                     /* int类型 >> 符号位保持不变 */
    int index = hash & (NR_CMD_TSIZE - 1);          /* 找到应该在的下标 */

    for (int i = 0;; i++) {
        if (hashTable[index].keyHashVal == 0) {     /* 该槽空闲。不会真有人的hash值为0吧，不会吧，不会吧？ */
            hashTable[index].cmdName = name;
            hashTable[index].keyHashVal = hash;
            hashTable[index].userTask = task;
//            printf("cmd:%s, index:%d, hash:%d.\n",name,index,hashTable[index].keyHashVal);
            return;
        }

        /**
         * 名槽有主，开始线性探测。
         * NR_CMD_TSIZE = NR_CMD *2，也就是说，表中有一半会是处于空闲状态的，所以我们不应该盲目地+1探测。
         * 而是，比如我们预定的命令数为8，那么hash表长度为16，将其看成高低两部分。
         * 当我们发现低位x冲突时，那么下一次应该检查对应的高位x+8。
         * 如果还是冲突，这次回来检查低位x+1，x+8+1。
         * 以0为例，它的检查顺序应该为，0，8，1，9，2，10...
         * 再配合位运算，有点无敌！！！
         *
         * 在java 10万级测试种显示，此种算法比+1要稳定。因为涉及cpu缓存，所以应该量级低更为有利一些。
         */
        index += NR_CMD + (i & 1);                  /* 每当来到奇数轮时+1，用位运算代替判断或者取模 */
        index &= (NR_CMD_TSIZE - 1);
    }

}

PUBLIC UserTask getTask(char *name, int cmdLen, HashTableNode hashTable[]) {
    if (cmdLen <= 0) return NO_USER_TASK;
    int hash = 0;
    for (int i = 0; i < cmdLen; i++) {
        hash = ((hash << 8) - hash) + name[i];
    }
    hash = hash ^ (hash >> 16);
    int index = hash & (NR_CMD_TSIZE - 1);          /* 找到应该在的下标 */

//    for (int i = 0; i < NR_CMD_TSIZE; ++i) {
//        printf("cmd:%s, index:%d, hash:%d.\n",hashTable[i].cmdName,i,hashTable[i].keyHashVal);
//    }

    for (int i = 0; i < NR_CMD_TSIZE; i++) {        /* 最多检查所有槽位 */
        if (hashTable[index].keyHashVal == 0)       /* 必定不存在 */
            return NO_USER_TASK;

        if (hashTable[index].keyHashVal == hash) {
            for (int j = 0; j < cmdLen; j++) {      /* 谨慎起见，继续检查名称 */
                if (name[j] != hashTable[index].cmdName[j])
                    return NO_USER_TASK;
            }
//            printf("cmd:%s, index:%d, hash:%d.\n",name,index,hash);
            return hashTable[index].userTask;
        }

        /* 检查下一个 */
        index += NR_CMD + (i & 1);                  /* 每当来到奇数轮时+1，用位运算代替判断或者取模 */
        index &= (NR_CMD_TSIZE - 1);
    }

    return NO_USER_TASK;
}
