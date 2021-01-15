//
// Created by 杜科 on 2020/10/22.
//
#include <core/kernel.h>
#include <stdarg.h>
#include "../include/stdio.h"

/**
 * kprintf函数，将字符串常量p_string格式化输出，用于系统任务和服务可用
 * 用户进程要与tty进行通信才能打印
 * @param p_string 待格式化的字符串常量
 * @param ... 可变参数
 * @return
 */
PUBLIC int kprintf(const char *p_string, ...) {
    va_list ap;
    int len;
    /**
     * 这里注意：必须是静态的不然这个函数每次调用都来个缓冲区，内核的内存根本不够它造，很快就溢出了！
     * 内核的打印不会超过两行，所以 160 就够了。
     */
    static char buf[160];

    /* 准备开始访问可变参数 */
    va_start(ap, p_string);

    /* 格式化字符串 */
    len = vsprintf(buf, p_string, ap);

//    /* 调用low_print函数打印格式化后的字符串 */
//    low_print(buf);
    for (int i = 0; i < len; i++) {
        out_char(&consoles[0], buf[i]);
    }

    /* 可变参数访问结束 */
    va_end(ap);
    return len;
}

PUBLIC int printf(const char *fmt, ...) {
    va_list ap;
    int len;
    char t_buf[1024];

    /* 准备开始访问可变参数 */
    va_start(ap, fmt);

    /* 格式化字符串 */
    len = vsprintf(t_buf, fmt, ap);

    int c = write(1, t_buf, len);

    assert(c == len);
//    if (c != len) panic("printf err\n", c);

    return len;
}

/* 只是简单的格式化字符串，但不进行输出 */
int sprintf(char *buf, const char *fmt, ...){
    va_list ap;

    va_start(ap, fmt);

    int rs = vsprintf(buf, fmt, ap);

    va_end(ap);
    return rs;
}