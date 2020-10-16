//
// Created by 杜科 on 2020/10/15.
//
/**
 * 一些库
 */

#ifndef AOS_SOME_LIB_H
#define AOS_SOME_LIB_H

#ifndef _ANSI_H
#include <ansi.h>
#endif

/* 关于 BSD. */
_PROTOTYPE(void swab, (char *_from, char *_to, int _count));
_PROTOTYPE(char *itoa, (int _n));
_PROTOTYPE(char *getpass, (const char *_prompt));

/* 关于 aos */

#endif //AOS_SOME_LIB_H
