//
// Created by 杜科 on 2021/1/27.
//

#include "stdio.h"
#include "../origin.h"
#include "stdlib.h"

int proc(int argc, char *argv[]) {
    alloc(get_pid());                   /* fork后父子共享这段内存 todo copy on write */
    if (argv[argc - 1][0] == '&') {     /* 如果当前是后台运行，那么将输出到文件tmp_out */
        while (TRUE) {

        };
    }

    while (TRUE);
    return 0;
}