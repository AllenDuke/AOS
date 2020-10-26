/**
 * 实现串处理函数，即头文件include/cstring.h中的功能
 * 这是c语言实现的，有几个用汇编实现的在同级目录下的string.asm里。
 */


#include "cstring.h"

/**
 * 比较两个字符串的前n个
 * @param s1
 * @param s2
 * @param n
 * @return
 */
int strncmp(register const char *s1, register const char *s2, register size_t n){
    if (n) {
        do {
            if (*s1 != *s2++){
                break;
            }
            if (*s1++ == '\0'){
                return 0;
            }
        } while (--n > 0);

        if (n > 0) {
            if (*s1 == '\0') return -1;
            if (*--s2 == '\0') return 1;
            return (unsigned char) *s1 - (unsigned char) *s2;
        }
    }
    return 0;
}

/**
 * 比较两个字符串
 * @param s1 串1
 * @param s2 串2
 * @return
 */
int strcmp(register const char *s1, register const char *s2){
    while (*s1 == *s2++) {
        if (*s1++ == '\0') {
            return 0;
        }
    }
    if (*s1 == '\0') return -1;
    if (*--s2 == '\0') return 1;
    return (unsigned char) *s1 - (unsigned char) *s2;
}




