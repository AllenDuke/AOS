//
// Created by 杜科 on 2021/2/4.
//
#include "stdio.h"
#include "stdlib.h"
#include "../origin.h"
#include "errno.h"

int help(int argc, char *argv[]) {

    HashTableNode *table=(HashTableNode *)argv[argc-1];
    printf("the cmd you can use as follow:\n");
    for (int i = 0; i < NR_CMD_TSIZE; ++i) {
        if(table[i].keyHashVal==0)continue;
        printf("%s\n",table[i].cmdName);
    }
    return 0;
}
