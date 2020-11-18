//
// Created by 杜科 on 2020/10/15.
//
/**
 * 这个头文件定义了许多基本的大小值，既有语言中的数据类型，如整数所占的位数，
 * 也有操作系统的限制，如文件名的长度。
 */
#ifndef AOS_LIMIT_H
#define AOS_LIMIT_H

/* 关于char类型（AOS中为8位，并带有符号）。 */
#define CHAR_BIT           8	/* # bits in a char */
#define CHAR_MIN        -128	/* minimum value of a char */
#define CHAR_MAX         127	/* maximum value of a char */
#define UCHAR_MAX        255	/* maximum value of an unsigned char */
#define MB_LEN_MAX         1	/* maximum length of a multibyte char */

/* 关于short类型（AOS中为16位）。 */
#define SHORT_MIN  (-32767-1)	/* minimum value of a short */
#define SHORT_MAX       32767	/* maximum value of a short */
#define USHORT_MAX     0xFFFF	/* maximum value of unsigned short */

/* 关于int类型（AOS中为32位）。 */
#define INT_MIN (-2147483647-1)	/* minimum value of a 32-bit int */
#define INT_MAX   2147483647	/* maximum value of a 32-bit int */
#define UINT_MAX  0xFFFFFFFF	/* maximum value of an unsigned 32-bit int */


/* 关于long类型(32位AOS)。 */
#define LONG_MIN (-2147483647L-1)/* minimum value of a long */
#define LONG_MAX  2147483647L	/* maximum value of a long */
#define ULONG_MAX 0xFFFFFFFFL	/* maximum value of an unsigned long */


#endif //AOS_LIMIT_H
