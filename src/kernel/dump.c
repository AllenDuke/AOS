/**
 * 该文件包含一些内核调试转储例程
 */
#include "core/kernel.h"

FORWARD char *proc_name(int proc_nr);

/* 转储进程信息 */
PUBLIC void dump_proc(void) {
    /* 为所有的进程显示基本的处理信息。 */
    register Process *target;
    static Process *old_proc = BEG_PROC_ADDR;
    int n = 0;

    kprintf("\n--nr- --eip- ---sp- flag -user --sys-- -base- -size- -recv- command\n");
    for (target = old_proc; target < END_PROC_ADDR; target++) {
        /* 空的进程请跳过 */
        if (is_empty_proc(target)) continue;
        if (++n > 20) break;
        if (target->pid < 0) {
            kprintf("#{%3d}", target->pid);
        } else {
            kprintf("%5d", target->pid);
        }
        kprintf(" %5lx %6lx %2x %6lus %6lus %5uK %5uK ",
                (unsigned long) target->regs.eip,
                (unsigned long) target->regs.esp,
                target->flags,
                tick2sec(target->userTime),
                tick2sec(target->sysTime),
                tick2sec(target->map.base),
                bytes2round_k(target->map.size));
        if (target->flags & RECEIVING) {
            kprintf("%-7.7s", proc_name(target->getFrom));
        } else if (target->flags & SENDING) {
            kprintf("S:%-5.5s", proc_name(target->sendTo));
        } else if (target->flags == CLEAN_MAP) {
            kprintf(" CLEAN ");
        }
        kprintf("%s.\n", target->name);
    }
    if (target == END_PROC_ADDR) target = BEG_PROC_ADDR; else kprintf("--more--\r");
    old_proc = target;
    kprintf("\n");
}

/* 得到进程名字 */
PRIVATE inline char *proc_name(int proc_nr) {
    if (proc_nr == ANY) return "ANY";
    return proc_addr(proc_nr)->name;
}

PUBLIC void dump_msg(const char *title, Message *m) {
    int packed = 0;
    kprintf("{%s}<0x%x>{%ssrc:%s(%d),%stype:%d,%s(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)%s}%s",  //, (0x%x, 0x%x, 0x%x)}",
            title,
            (int) m,
            packed ? "" : "\n        ",
            proc_addr(m->source)->name,
            m->source,
            packed ? " " : "\n        ",
            m->type,
            packed ? " " : "\n        ",
            m->m2_i1,
            m->m2_i2,
            m->m2_i3,
            (int) m->m2_l1,
            (int) m->m2_l2,
            (int) m->m2_p1,
            packed ? "" : "\n",
            packed ? "" : "\n"/* , */
    );
}