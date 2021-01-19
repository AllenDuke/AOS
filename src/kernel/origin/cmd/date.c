//
// Created by 杜科 on 2021/1/18.
//

#include "stdio.h"
#include "stdlib.h"
#include "../origin.h"

int date(int argc, char *argv[]) {
    printf("current date is: %ld\n", get_time());
    return 0;
}