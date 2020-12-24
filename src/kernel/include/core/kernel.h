//
// Created by 杜科 on 2020/10/16.
//
/**
 * 该文件包含内核所需要的所有定义，通常.c文件中只需要引入kernel.h即可
 */

/**
 * _POSIX_SOURCE是POSIX标准自行定义的一个特征检测宏。
 * 作用是保证所有POSIX要求的符号和那些显式地允许但并不要求的符号将是可见的，
 * 同时隐藏掉任何POSIX非官方扩展的附加符号。
 */
#define _POSIX_SOURCE      1

/* 宏_AOS将为AOS所定义的扩展而"重载_POSIX_SOURCE"的作用 */
#define _AOS               1

/* 在编译系统代码时，如果要作与用户代码不同的事情，比如改变错误码的符号，则可以对_SYSTEM宏进行测试 */
#define _SYSTEM            1	/* tell headers that this is the kernel */

#include "config.h"             /* 这个头文件应该第一个引入，做一些检查和设定，但不是 */
#include "constant.h"
#include "../errno.h"
#include "../types.h"
#include "times.h"
#include "../cstring.h"
#include "struct_type.h"
#include "protect.h"
#include "message.h"
#include "process.h"
#include "limit.h"
#include "console.h"
#include "tty.h"
#include "lib.h"
#include "mm.h"
#include "prototype.h"
#include "global.h"

