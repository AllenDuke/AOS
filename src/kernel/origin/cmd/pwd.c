//
// Created by 杜科 on 2021/1/18.
//

#include "stdio.h"
#include "../origin.h"

int pwd(int argc, char *argv[]) {
    if (argv[argc - 1][0] == '&') {     /* 如果当前是后台运行，那么将输出到文件tmp_out */
        int out = open("/tmp_out", O_RDWR);
        char buf[256];
        sprintf(buf, "/\n");
        int i = 0;
        while (buf[i] != '0') i++;
        write(out, buf, i);
        close(out);
        return 0;
    }

//    if(fork()==0)
    printf("/\n");
    return 0;
}