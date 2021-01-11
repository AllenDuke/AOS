//
// Created by 杜科 on 2020/10/16.
//
/**
 * 包含串处理函数原型
 */

#ifndef AOS_CSTRING_H
#define AOS_CSTRING_H

#include "types.h"

int strncmp(register const char *p_s1, register const char *p_s2, register size_t n);
int strcmp(register const char *p_s1, register const char *p_s2);
int memcmp(const void *p_s1, const void *p_s2, size_t n);

unsigned int strlen(char* p_str);
char* strcpy(char* p_dst, char* p_src);
void memset(void* p_dst, char ch, int size);
void*	memcpy(void* p_dst, void* p_src, int size);

#endif //AOS_CSTRING_H
