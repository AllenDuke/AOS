/**
 * 系统时间相关
 */

#ifndef AOS_TIMES_H
#define AOS_TIMES_H

#ifndef _CLOCK_T
#define _CLOCK_T
typedef long clock_t;		/* 系统时钟计时单位 */
#endif

#include <core/cmos.h>

/* 系统硬件实时时间
 * 通过 CMOS 中获取的 RTC 时间
 */
typedef struct rtc_time {
    unsigned short year;         /* year, eg. 1997 */
    unsigned char month;         /* mouths of year */
    unsigned short day;          /* days of month, eg. 9,它只是为了对齐，所以使用了 16 位 */
    unsigned char hour;          /* hours of day */
    unsigned char minute;        /* minutes of day */
    unsigned char second;        /* seconds of day */
} RTCTime_t;

/* BDC 码 转 十进制 */
#define  bcd2dec(x)      ( (x >> 4) * 10 + (x & 0xf) )

#endif //_FLYANX_TIMES_H
