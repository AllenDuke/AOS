#include <stdio.h>
#include <stdarg.h>

/* 只是简单的格式化字符串，但不进行输出 */
int sprintf(char *buf, const char *fmt, ...){
    va_list ap;

    /* 准备访问可变参数 */
    va_start(ap, fmt);

    /* 格式化字符串 */
    int rs = vsprintf(buf, fmt, ap);

    /* 访问可变参数结束 */
    va_end(ap);
    return rs;
}
