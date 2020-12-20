//
// Created by 杜科 on 2020/10/24.
//

#include "include/core/kernel.h"

/**
 * 内核遇到了不可恢复的异常或错误，立即准备进入宕机状态
 * @param msg 错误信息
 * @param error_num
 */
PUBLIC void panic(const char *p_msg, int errorNum) {
    if (p_msg != NIL_PTR) {
        kprintf("AOS kernel is panicky for: %s !\n", p_msg);
        if (errorNum != PANIC_ERR_NUM)
            kprintf("panic error num: 0x%x !\n", errorNum);
    }
    /* 好了，可以宕机了 */
    level0(cpu_halt);
}

/**
 * 断言失败后的处理
 * @param file 断言失败代码所在文件
 * @param line 断言失败代码所在文件的行数
 * @param p_srcCode 断言源代码
 */
PUBLIC void bad_assertion(char *file, int line, char *p_srcCode) {
    kprintf("panic at file://%s(%d): assertion \"%s\" failed\n", file, line, p_srcCode);
    panic("bad assertion", PANIC_ERR_NUM);
}

/**
 * 断定比较失败后的处理
 * @param file 断定比较失败代码所在文件
 * @param line 断定比较失败代码所在文件的行数
 * @param lhs 左边的比较数
 * @param p_srcCode 断定比较源代码
 * @param rhs 右边的比较数
 */
PUBLIC void bad_compare(char *file, int line, int lhs, char *p_srcCode, int rhs) {
    kprintf("* panic at file://%s(%d): compare \"%s\" failed\n", file, line, p_srcCode);
    panic("bad compare", PANIC_ERR_NUM);
}
