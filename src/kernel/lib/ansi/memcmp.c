/**
 * ansi标准函数memcmp实现。
 */



#include <types.h>

/**
 * 比较两个内存区域的前n个字节
 * @param p_s1
 * @param p_s2
 * @param n
 * @return
 */
int memcmp(const void *p_s1, const void *p_s2, size_t n){
    if ((p_s1 == 0) || (p_s2 == 0)) return (p_s1 - p_s2);

    register const char *p1 = p_s1;
    register const char *p2 = p_s2;
    for (int i = 0; i < n; i++,p1++,p2++) {
        if (*p1 != *p2) return (*p1 - *p2);
    }
    return 0;
}

