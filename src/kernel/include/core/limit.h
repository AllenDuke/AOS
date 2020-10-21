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

/* POSIX P1003.1标准要求的最小尺寸（表2-3）。 */
#ifdef _POSIX_SOURCE		/* these are only visible for POSIX */

#define _POSIX_ARG_MAX    4096	/* exec() may have 4K worth of args */
#define _POSIX_CHILD_MAX     6	/* a process may have 6 children */
#define _POSIX_LINK_MAX      8	/* a file may have 8 links */
#define _POSIX_MAX_CANON   255	/* size of the canonical input queue */
#define _POSIX_MAX_INPUT   255	/* you can type 255 chars ahead */
#define _POSIX_NAME_MAX     14	/* a file name may have 14 chars */
#define _POSIX_NGROUPS_MAX   0	/* supplementary group IDs are optional */
#define _POSIX_OPEN_MAX     16	/* a process may have 16 files open */
#define _POSIX_PATH_MAX    255	/* a pathname may contain 255 chars */
#define _POSIX_PIPE_BUF    512	/* pipes writes of 512 bytes must be atomic */
#define _POSIX_STREAM_MAX    8	/* at least 8 FILEs can be open at once */
#define _POSIX_TZNAME_MAX    3	/* time zone names can be at least 3 chars */
#define _POSIX_SSIZE_MAX 32767	/* read() must support 32767 byte reads */

#endif /* _POSIX_SOURCE */

#endif //AOS_LIMIT_H
