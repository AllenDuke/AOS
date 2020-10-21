//
// Created by 杜科 on 2020/10/16.
//
/**
 * 包含串处理函数原型
 */

#ifndef _STRING_H
#define _STRING_H

#define NULL    ((void *)0)

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;	/* 由sizeof返回的类型 */
#endif /*_SIZE_T */

#ifndef _ANSI_H
#include "ansi.h"
#endif

_PROTOTYPE( void *memchr, (const void *_s, int _c, size_t _n)		);
_PROTOTYPE( int memcmp, (const void *_s1, const void *_s2, size_t _n)	);
_PROTOTYPE( void *memcpy, (void *_s1, const void *_s2, size_t _n)	);
_PROTOTYPE( void *memmove, (void *_s1, const void *_s2, size_t _n)	);
_PROTOTYPE( void *memset, (void *_s, int _c, size_t _n)			);
_PROTOTYPE( char *strcat, (char *_s1, const char *_s2)			);
_PROTOTYPE( char *strchr, (const char *_s, int _c)			);
_PROTOTYPE( int strncmp, (const char *_s1, const char *_s2, size_t _n)	);
_PROTOTYPE( int strcmp, (const char *_s1, const char *_s2)		);
_PROTOTYPE( int strcoll, (const char *_s1, const char *_s2)		);
_PROTOTYPE( char *strcpy, (char *_s1, const char *_s2)			);
_PROTOTYPE( size_t strcspn, (const char *_s1, const char *_s2)		);
_PROTOTYPE( char *strerror, (int _errnum)				);
_PROTOTYPE( size_t strlen, (const char *_s)				);
_PROTOTYPE( char *strncat, (char *_s1, const char *_s2, size_t _n)	);
_PROTOTYPE( char *strncpy, (char *_s1, const char *_s2, size_t _n)	);
_PROTOTYPE( char *strpbrk, (const char *_s1, const char *_s2)		);
_PROTOTYPE( char *strrchr, (const char *_s, int _c)			);
_PROTOTYPE( size_t strspn, (const char *_s1, const char *_s2)		);
_PROTOTYPE( char *strstr, (const char *_s1, const char *_s2)		);
_PROTOTYPE( char *strtok, (char *_s1, const char *_s2)			);
_PROTOTYPE( size_t strxfrm, (char *_s1, const char *_s2, size_t _n)	);

/* AOS特有的，自己实现的 */
#ifdef _AOS

#endif

#endif //_STRING_H
