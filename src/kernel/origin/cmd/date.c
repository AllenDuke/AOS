//
// Created by 杜科 on 2021/1/18.
//

#include "stdio.h"
#include "stdlib.h"
#include "../origin.h"

int date(int argc, char *argv[]) {
    if (argv[argc - 2][0] == '&') {
        int out = *argv[argc-1];
        char buf[256];
        sprintf(buf, "current date is: %ld\n", get_time());
        int i = 0;
        while (buf[i] != '0') i++;
        write(out, buf, i);
        return 0;
    }
    printf("current date is: %ld\n", get_time());
    return 0;
}