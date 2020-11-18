//
// Created by 杜科 on 2020/10/16.
//
/**
 * 包含串处理函数原型
 */

#ifndef _CSTRING_H
#define _CSTRING_H

int strncmp(register const char *s1, register const char *s2, register size_t n);
int strcmp(register const char *s1, register const char *s2);
int memcmp(const void *s1, const void *s2, size_t n);

unsigned int strlen(char* p_str);
char* strcpy(char* p_dst, char* p_src);
void memset(void* p_dst, char ch, int size);
void*	memcpy(void* p_dst, void* p_src, int size);

#endif //_CSTRING_H
