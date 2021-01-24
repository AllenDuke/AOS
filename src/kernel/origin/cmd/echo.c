//
// Created by 杜科 on 2021/1/24.
//

#include "stdio.h"
#include "../origin.h"

int echo(int argc, char *argv[]) {
    int i;
    for (i = 1; i < argc; i++)
        printf("%s%s", i == 1 ? "" : " ", argv[i]);
    printf("\n");

    return 0;
}