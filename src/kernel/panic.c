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
        kprintf("%c AOS kernel is panicky for: %s !\n",MAG_CH_PANIC, p_msg);
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
PUBLIC void assertion_failure(char *exp, char *file, char *base_file, int line) {
    kprintf("%c  assert(%s) failed: file: %s, base_file: %s, ln%d",
           MAG_CH_ASSERT,
           exp, file, base_file, line);

    panic("assertion_failure", PANIC_ERR_NUM);
}