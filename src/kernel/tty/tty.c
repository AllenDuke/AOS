//
// Created by 杜科 on 2020/12/8.
//

#include <core/console.h>
#include <core/global.h>
#include "core/keyboard.h"
#include "stdio.h"
/* 本文件内函数声明 */
PRIVATE void init_tty(TTY *p_tty);

PRIVATE void tty_dev_read(TTY *p_tty);

PRIVATE void tty_dev_write(TTY *p_tty);

PRIVATE void put_key(TTY *p_tty, u32_t key);

PRIVATE void tty_do_read(TTY *tty, Message *msg);

PRIVATE void tty_do_write(TTY *tty, Message *msg);

PRIVATE Message ttyMsg;

PUBLIC void tty_task() {

    TTY *p_tty;

    init_keyboard();

    /* 初始化收发件箱 */
    io_box(&ttyMsg);

    for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++) {
        init_tty(p_tty);
    }

    select_console(0);

    while (1) {

        for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++) {
            do {
                tty_dev_read(p_tty);
                tty_dev_write(p_tty);
            } while (p_tty->inBufCount);
        }
        receive(ANY, &ttyMsg);

        int src = ttyMsg.source;
//        assert(src != TASK_TTY);
        if (src == TTY_TASK) panic("dead lock found, tty can not send msg to tty\n", PANIC_ERR_NUM);

        TTY *ptty = &ttys[ttyMsg.DEVICE];

        switch (ttyMsg.type) {
            case DEVICE_OPEN:
                ttyMsg.type = SYSCALL_RET;
                send(src, &ttyMsg);
                break;
            case DEVICE_READ:
                tty_do_read(ptty, &ttyMsg);
                break;
            case DEVICE_WRITE:
                tty_do_write(ptty, &ttyMsg);
                break;
            case HARD_INT:
                /**
                 * waked up by clock_handler -- a key was just pressed
                 * @see clock_handler() inform_int()
                 */
//                key_pressed = 0;
                continue;
            default:
                kprintf("TTY::unknown ttyMsg:%s", &ttyMsg);
                break;
        }
    }
}

PUBLIC void in_process(u32_t key, TTY *p_tty) {
    if (!(key & FLAG_EXT)) {
        put_key(p_tty, key);
    } else {
        int raw_code = key & MASK_RAW;
        switch (raw_code) {
            case ENTER:
                put_key(p_tty, '\n');
                break;
            case BACKSPACE:
                put_key(p_tty, '\b');
                break;
            case UP:
                if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {    /* Shift + Up 上滚5行 */
                    scroll_screen(p_tty->p_console, SCROLL_SCREEN_UP);
                }
                break;
            case DOWN:
                if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {    /* Shift + Down */
                    scroll_screen(p_tty->p_console, SCROLL_SCREEN_DOWN);
                }
                break;
            case F1:
            case F2:
            case F3:
            case F4:
            case F5:
            case F6:
            case F7:
            case F8:
            case F9:
            case F10:
            case F11:
            case F12:
                if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {    /* Alt + F1~F12 */
                    select_console(raw_code - F1);
                }
                break;
            default:
                break;
        }
    }

}

PUBLIC bool_t is_cur_console(CONSOLE *p_con) {
    return (p_con == &consoles[nrCurConsole]);
}

PRIVATE void init_tty(TTY *p_tty) {
    p_tty->inBufCount = 0;
    p_tty->p_inBufHead = p_tty->p_inBufTail = p_tty->inBuf;

    init_screen(p_tty);
}

PRIVATE void tty_dev_read(TTY *p_tty) {
    if (is_cur_console(p_tty->p_console)) keyboard_read(p_tty);
}

PRIVATE char cmd[TTY_IN_BYTES];
PRIVATE u32_t cmdLen = 0;
PRIVATE char *cmd_map[NR_CMDS] = {
        "",
        "",
        "pwd",
        "date",
        "",
        "",
        "",
        "",
};

PRIVATE void exec_cmd();

PRIVATE void tty_dev_write(TTY *tty) {
    while (tty->inBufCount) {
        char ch = *(tty->p_inBufTail);
        tty->p_inBufTail++;
        if (tty->p_inBufTail == tty->inBuf + TTY_IN_BYTES)
            tty->p_inBufTail = tty->inBuf;
        tty->inBufCount--;

        if (tty->tty_left_cnt) {
            if (ch >= ' ' && ch <= '~') { /* printable */
                out_char(tty->p_console, ch);
                void * p = tty->tty_req_buf +
                           tty->tty_trans_cnt;
                phys_copy((void *)proc_vir2phys(proc_addr(TTY_TASK), &ch),p,  1);
                tty->tty_trans_cnt++;
                tty->tty_left_cnt--;
            }
            else if (ch == '\b' && tty->tty_trans_cnt) {
                out_char(tty->p_console, ch);
                tty->tty_trans_cnt--;
                tty->tty_left_cnt++;
            }

            if (ch == '\n' || tty->tty_left_cnt == 0) {
                out_char(tty->p_console, '\n');
                Message msg;
                msg.type = RESUME_PROC;
                msg.PROC_NR = tty->tty_procnr;
                msg.COUNT = tty->tty_trans_cnt;
                send(tty->tty_caller, &msg);
                tty->tty_left_cnt = 0;
            }
        }
    }
}

PRIVATE void exec_cmd() {
    if (cmdLen == 0) return;
    int hash = 0;

    /**
     * 类似java String hash值计算，不直接采取31是因为cmd一般比较短，而31又比较小，
     * 127这个质数比较大，可尽可能地使得hash的高位有值
     * hash=hash*127+cmd[i]
     */
    for (int i = 0; i < cmdLen; i++) {
        hash = ((hash << 8) - hash) + cmd[i];
    }

    /**
     * 扰动函数
     * 同java HashMap hash()
     * hash值的高16位为原hash的高16位，低16位为原hash高16位与原hash低16位异或后的结果。
     * 原因：因为数组的长度为2的次方数，所以最终计算下标时，使用(NR_CMDS-1)&hash，
     * 这样一来，因为(NR_CMDS-1)的高位总是0，hash的高位没有参与到运算，为了进一步降低冲突，应使得高位也参与运算
     * 在权衡质量与速度后，选择了这种方式。成本不高，质量也不错。
     */
    hash = hash ^ (hash >> 16); /* int类型 >> 符号位保持不变 */
    int index = hash & (NR_CMDS - 1);

    char *pre = cmd_map[index];
    for (int i = 0; i < cmdLen; i++) {
        if (cmd[i] != *pre || *pre == '\0') {
            kprintf("no such cmd:%s, hash:%d, index:%d\n", cmd, hash, index);
            return;
        }
        pre++;
    }
    //todo fork出一条进程去执行cmd
    switch (index) {
        case 0: {
            kprintf("default cmd\n");
            break;
        }
        case 1: {
            kprintf("default cmd\n");
            break;
        }
        case 2: {
            kprintf("/\n");
            break;
        }
        case 3: {
            ttyMsg.source = TTY_TASK;
            ttyMsg.type = GET_TIME;
            send_rec(CLOCK_TASK, &ttyMsg);
            kprintf("current date is: %d\n\r", ttyMsg.CLOCK_TIME);
        }
        case 4: {
            kprintf("default cmd\n");
            break;
        }
        case 5: {
            kprintf("default cmd\n");
            break;
        }
        case 6: {
            kprintf("default cmd\n");
            break;
        }
        case 7: {
            kprintf("default cmd\n");
            break;
        }
    }
}

PRIVATE void put_key(TTY *p_tty, u32_t key) {
    if (p_tty->inBufCount < TTY_IN_BYTES) {
        *(p_tty->p_inBufHead) = key;
        p_tty->p_inBufHead++;
        if (p_tty->p_inBufHead == p_tty->inBuf + TTY_IN_BYTES) {
            p_tty->p_inBufHead = p_tty->inBuf;
        }
        p_tty->inBufCount++;
    }
}

/*****************************************************************************
 *                                tty_do_read
 *****************************************************************************/
/**
 * Invoked when task TTY receives DEV_READ message.
 *
 * @note The routine will return immediately after setting some members of
 * TTY struct, telling FS to suspend the proc who wants to read. The real
 * transfer (tty buffer -> proc buffer) is not done here.
 *
 * @param tty  From which TTY the caller proc wants to read.
 * @param msg  The MESSAGE just received.
 *****************************************************************************/
PRIVATE void tty_do_read(TTY *tty, Message *msg) {
    /* tell the tty: */
    tty->tty_caller = msg->source;  /* who called, usually FS */
    tty->tty_procnr = msg->PROC_NR; /* who wants the chars */
    tty->tty_req_buf = proc_vir2phys(proc_addr(tty->tty_procnr),
                                     msg->BUF);/* where the chars should be put */
    tty->tty_left_cnt = msg->COUNT; /* how many chars are requested */
    tty->tty_trans_cnt = 0; /* how many chars have been transferred */

    msg->type = SUSPEND_PROC;
    msg->COUNT = tty->tty_left_cnt;
    send(tty->tty_caller, msg);
}


/*****************************************************************************
 *                                tty_do_write
 *****************************************************************************/
/**
 * Invoked when task TTY receives DEV_WRITE message.
 *
 * @param tty  To which TTY the calller proc is bound.
 * @param msg  The MESSAGE.
 *****************************************************************************/
PRIVATE void tty_do_write(TTY *tty, Message *msg) {
    char buf[TTY_OUT_BUF_LEN];
    char *p = (char *) proc_vir2phys(proc_addr(msg->PROC_NR),
                                     msg->BUF);;
    int i = msg->COUNT;
    int j;

    while (i) {
        int bytes = MIN(TTY_OUT_BUF_LEN, i);
        phys_copy((void *) p,proc_vir2phys(proc_addr(TTY_TASK), buf), bytes);
        for (j = 0; j < bytes; j++)
            out_char(tty->p_console, buf[j]);
        i -= bytes;
        p += bytes;
    }

    msg->type = SYSCALL_RET;
    send(msg->source, msg);
}