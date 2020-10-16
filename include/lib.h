//
// Created by 杜科 on 2020/10/16.
//

#ifndef AOS_LIB_H
#define AOS_LIB_H

/*
 * _POSIX_SOURCE是POSIX标准自行定义的一个特征检测宏。
 * 作用是保证所有POSIX要求的符号和那些显式地允许但并不要求的符号将是可见的，
 * 同时隐藏掉任何POSIX非官方扩展的附加符号。
 */
#define _POSIX_SOURCE       1
/* 宏_FLYANX将为FLYANX所定义的扩展而"重载_POSIX_SOURCE"的作用 */
#define _AOS             1

/* 引入库应该所需要的头文件 */
#include <os/config.h>
#include <sys/types.h>
#include <limit.h>
#include <errno.h>
#include <ansi.h>

#include <os/constant.h>
#include <os/struct_type.h>
#include <os/sys_call_num.h>

#define MM                  0       /* 内存管理器 */
#define FS                  1       /* 文件系统 */
#define FLY                 2       /* 飞彦拓展器，其他调用都在这处理 */

#endif //AOS_LIB_H
