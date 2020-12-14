/**
 * 该文件包含一些内核调试转储例程
 */
#include "core/kernel.h"

FORWARD char *proc_name(int proc_nr);

/* 转储进程信息 */
PUBLIC void proc_dump(void) {
    /* 为所有的进程显示基本的处理信息。 */
    register Process *target;
    static Process *old_proc = BEG_PROC_ADDR;
    int n = 0;

    printf("\n--nr- --eip- ---sp- flag -user --sys-- -base- -size- -recv- command\n");
    for (target = old_proc; target < END_PROC_ADDR; target++) {
        /* 空的进程请跳过 */
        if (is_empty_proc(target)) continue;
        if (++n > 20) break;
        if (target->logicNum < 0) {
            printf("#{%3d}", target->logicNum);
        } else {
            printf("%5d", target->logicNum);
        }
        printf(" %5lx %6lx %2x %6lus %6lus %5uK %5uK ",
               (unsigned long) target->regs.eip,
               (unsigned long) target->regs.esp,
               target->flags,
               tick2sec(target->userTime),
               tick2sec(target->sysTime),
               tick2sec(target->map.base),
               bytes2round_k(target->map.size));
        if (target->flags & RECEIVING) {
            printf("%-7.7s", proc_name(target->getFrom));
        } else if (target->flags & SENDING) {
            printf("S:%-5.5s", proc_name(target->sendTo));
        } else if (target->flags == CLEAN_MAP) {
            printf(" CLEAN ");
        }
        printf("%s\n", target->name);
    }
    if (target == END_PROC_ADDR) target = BEG_PROC_ADDR; else printf("--more--\r");
    old_proc = target;
    printf("\n");
}


/* 转储进程内存影响信息 */
PUBLIC void map_dump(void) {
    /* 提供详细的内存使用信息。 */
    register Process *target;
    static Process *old_proc = cproc_addr(HARDWARE);
    int n = 0;

    printf("\nPROC  -NAME-  -----BASE-----  -SIZE-\n");
    for (target = old_proc; target < END_PROC_ADDR; target++) {
        if (is_empty_proc(target)) continue;    /* 空进程跳过 */
        if (++n > 20) break;
        printf("%3d %s  %12xB  %5uK\n",
               target->logicNum,
               target->name,
               target->map.base,
               bytes2round_k(target->map.size));
    }
    if (target == END_PROC_ADDR) target = cproc_addr(HARDWARE); else printf("--more--\r");
    old_proc = target;
    printf("\n");
}

/* 得到进程名字 */
PRIVATE inline char *proc_name(int proc_nr) {
    if (proc_nr == ANY) return "ANY";
    return proc_addr(proc_nr)->name;
}