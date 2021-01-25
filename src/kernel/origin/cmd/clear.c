//
// Created by 杜科 on 2021/1/24.
//

#include "stdlib.h"
#include "../origin.h"

/* 与linux不同，这里清理当前console所属的整段显存 */
int clear(int argc, char *argv[]) {
    clean_console();
    return 0;
}