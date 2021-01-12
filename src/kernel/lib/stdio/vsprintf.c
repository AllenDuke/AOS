//
// Created by 杜科 on 2021/1/11.
//

#include "core/kernel.h"
#include "stdarg.h"

#define isdigit(c)    ((unsigned) ((c) - '0') <  (unsigned) 10)

/**
 * 格式化一个字符串，并将格式化完成的字符串拷贝到缓存区
 * @param buf
 * @param p_string
 * @param p_arg
 * @return
 */
PUBLIC int vsprintf(char *buf, const char *p_string, char *p_arg) {
    int c;
    enum {
        LEFT, RIGHT
    } adjust;
    enum {
        LONG, INT
    } intsize;
    int fill;
    int width, max, len, base;
    static char X2C_tab[] = "0123456789ABCDEF";
    static char x2c_tab[] = "0123456789abcdef";
    char *x2c;
    char *p;
    long i;
    unsigned long u;
    char temp[8 * sizeof(long) / 3 + 2];
    int bufLen = 0;

    /* 只要还没有访问到字符串的结束符0，就继续 */
    while ((c = *p_string++) != 0) {
        if (c != '%') {
            /* 普通字符，将其回显 */
            *buf = c;
            buf++;
            bufLen++;
            continue;
        }

        /* 格式说明符，格式为：
         * ％[adjust] [fill] [width] [.max]keys
         */
        c = *p_string++;

        adjust = RIGHT;
        if (c == '-') {
            adjust = LEFT;
            c = *p_string++;
        }

        fill = ' ';
        if (c == '0') {
            fill = '0';
            c = *p_string++;
        }

        width = 0;
        if (c == '*') {
            /* 宽度被指定为参数，例如 %*d。 */
            width = (int) va_arg(p_arg, int);
            c = *p_string++;
        } else if (isdigit(c)) {
            /* 数字表示宽度，例如 %10d。 */
            do {
                width = width * 10 + (c - '0');
            } while (isdigit(c = *p_string++));
        }

        max = INT_MAX;
        if (c == '.') {
            /* 就要到最大字段长度了 */
            if ((c = *p_string++) == '*') {
                max = (int) va_arg(p_arg, int);
                c = *p_string++;
            } else if (isdigit(c)) {
                max = 0;
                do {
                    max = max * 10 + (c - '0');
                } while (isdigit(c = *p_string++));
            }
        }

        /* 将一些标志设置为默认值 */
        x2c = x2c_tab;
        i = 0;
        base = 10;
        intsize = INT;
        if (c == 'l' || c == 'L') {
            /* “Long”键，例如 %ld。 */
            intsize = LONG;
            c = *p_string++;
        }
        if (c == 0) break;

        switch (c) {
            /* 十进制 */
            case 'd':
                i = intsize == LONG ? (long) va_arg(p_arg, long)
                                    : (long) va_arg(p_arg, int);
                u = i < 0 ? -i : i;
                goto int2ascii;

                /* 八进制 */
            case 'o':
                base = 010;
                goto getint;

                /* 指针，解释为%X 或 %lX。 */
            case 'p':
                if (sizeof(char *) > sizeof(int)) intsize = LONG;

                /* 十六进制。 %X打印大写字母A-F，而不打印%lx。 */
            case 'X':
                x2c = X2C_tab;
            case 'x':
                base = 0x10;
                goto getint;

                /* 无符号十进制 */
            case 'u':
            getint:
                u = intsize == LONG ? (unsigned long) va_arg(p_arg, unsigned long)
                                    : (unsigned long) va_arg(p_arg, unsigned int);
            int2ascii:
                p = temp + sizeof(temp) - 1;
                *p = 0;
                do {
                    *--p = x2c[(int) (u % base)];
                } while ((u /= base) > 0);
                goto string_length;

                /* 一个字符 */
            case 'c':
                p = temp;
                *p = (int) va_arg(p_arg, int);
                len = 1;
                goto string_print;

                /* 只是一个百分号 */
            case '%':
                p = temp;
                *p = '%';
                len = 1;
                goto string_print;

                /* 一个字符串，其他情况将加入这里。 */
            case 's':
                p = va_arg(p_arg, char *);

            string_length:
                for (len = 0; p[len] != 0 && len < max; len++) {}

            string_print:
                width -= len;
                if (i < 0) width--;
                if (fill == '0' && i < 0) {
                    *buf++ = '-';
                    bufLen++;
                }
                if (adjust == RIGHT) {
                    while (width > 0) {
                        *buf = fill;
                        buf++;
                        bufLen++;
                        width--;
                    }
                }
                if (fill == ' ' && i < 0) *buf++ = '-';
                while (len > 0) {
                    *buf = (unsigned char) *p++;
                    buf++;
                    bufLen++;
                    len--;
                }
                while (width > 0) {
                    *buf = fill;
                    buf++;
                    bufLen++;
                    width--;
                }
                break;

                /* 无法识别的格式键，将其回显。 */
            default:
                *buf = '%';
                *buf = c;
                buf += 2;
                bufLen += 2;
        }
    }

    /* 字符串已经格式化完毕，最后将结尾设置为字符串结束符0 */
    *buf++ = 0;
    return bufLen;
}