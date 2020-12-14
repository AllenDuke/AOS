/**
 * 实现串处理函数，即头文件include/cstring.h中的功能
 * 这是c语言实现的，有几个用汇编实现的在同级目录下的string.asm里。
 */
#include <types.h>

/**
 * 比较两个字符串的前n个
 * @param p_s1
 * @param p_s2
 * @param n
 * @return
 */
int strncmp(register const char *p_s1, register const char *p_s2, register size_t n){
    if (n) {
        do {
            if (*p_s1 != *p_s2++) break;
            if (*p_s1++ == '\0') return 0;
        } while (--n > 0);

        if (n > 0) {
            if (*p_s1 == '\0') return -1;
            if (*--p_s2 == '\0') return 1;
            return (unsigned char) *p_s1 - (unsigned char) *p_s2;
        }
    }
    return 0;
}

/**
 * 比较两个字符串
 * @param p_s1 串1
 * @param p_s2 串2
 * @return
 */
int strcmp(register const char *p_s1, register const char *p_s2){
    while (*p_s1 == *p_s2++) {
        if (*p_s1++ == '\0') return 0;
    }
    if (*p_s1 == '\0') return -1;
    if (*--p_s2 == '\0') return 1;
    return (unsigned char) *p_s1 - (unsigned char) *p_s2;
}




