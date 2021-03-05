//
// Created by 杜科 on 2020/12/8.
//

#include "core/kernel.h"
#include "core/keyboard.h"
#include "core/keymap.h"

PRIVATE KBInPut kbInPut;

PRIVATE bool_t codeWithE0 = FALSE;
PRIVATE bool_t shiftL;      /* l shift state	*/
PRIVATE bool_t shiftR;      /* r shift state	*/
PRIVATE bool_t altL;        /* l alt state		*/
PRIVATE bool_t altR;        /* r left state		*/
PRIVATE bool_t ctrlL;       /* l ctrl state		*/
PRIVATE bool_t ctrlR;       /* l ctrl state		*/
PRIVATE int column = 0;     /* keyrow[column] 将是 keymap 中某一个值 */
PRIVATE bool_t capsLock;        /* Caps Lock		*/
PRIVATE bool_t numLock;         /* Num Lock		*/
PRIVATE bool_t scrollLock;      /* Scroll Lock		*/

/* 本文件内函数声明 */
PRIVATE u8_t get_byte_from_kb_buf();

PRIVATE void set_leds();

PRIVATE void kb_wait();

PRIVATE void kb_ack();

PRIVATE Message msg;
PRIVATE u8_t initCount = 0;

PUBLIC int keyboard_handler(int irq) {
    u8_t scanCode = in_byte(KB_DATA);

    if (kbInPut.count < KB_IN_BYTES) {
        *(kbInPut.p_head) = scanCode;
        kbInPut.p_head++;
        if (kbInPut.p_head == kbInPut.buf + KB_IN_BYTES) {
            kbInPut.p_head = kbInPut.buf;
        }
        kbInPut.count++;
    }
    if (initCount >= 1) { /* todo 初始时，莫名其妙产生的一次键盘中断 */
//        kprintf("scanCode:%d.\n",scanCode);
//        ready(proc_addr(TTY_TASK));
//        aos_unpark(TTY_TASK);
          interrupt(TTY_TASK);
//        unpark(TTY_TASK); /* unpark是系统调用，在中断中嵌套调用会页异常 */
    } else initCount++;

    return ENABLE;
}


PUBLIC void init_keyboard() {
    kbInPut.count = 0;
    kbInPut.p_head = kbInPut.p_tail = kbInPut.buf;

    capsLock = 0;
    numLock = 1;
    scrollLock = 0;

    set_leds();

    put_irq_handler(KEYBOARD_IRQ, keyboard_handler);    /* 设定键盘中断处理程序 */
    enable_irq(KEYBOARD_IRQ);                /* 开键盘中断 */
}

PUBLIC int kb_in_count() {
    return kbInPut.count;
}

PUBLIC void keyboard_read(TTY *p_tty) {
    u8_t scanCode;
    bool_t make;        /* TRUE : make  */
    /* FALSE: break */
    u32_t key = 0;    /* 用一个整型来表示一个键。 */
    /* 比如，如果 Home 被按下，则 key 值将为定义在 keyboard.h 中的 'HOME'。*/
    u32_t *keyrow;        /* 指向 keymap[] 的某一行 */

    if (kbInPut.count > 0) {
        codeWithE0 = FALSE;
        scanCode = get_byte_from_kb_buf();

        /* 下面开始解析扫描码 */
        if (scanCode == 0xE1) {
            int i;
            u8_t pauseBreakScanCode[] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};
            bool_t isPauseBreak = TRUE;
            for (i = 1; i < 6; i++) {
                if (get_byte_from_kb_buf() != pauseBreakScanCode[i]) {
                    isPauseBreak = FALSE;
                    break;
                }
            }
            if (isPauseBreak) {
                key = PAUSEBREAK;
            }
        } else if (scanCode == 0xE0) {
            scanCode = get_byte_from_kb_buf();

            /* PrintScreen 被按下 */
            if (scanCode == 0x2A) {
                if (get_byte_from_kb_buf() == 0xE0) {
                    if (get_byte_from_kb_buf() == 0x37) {
                        key = PRINTSCREEN;
                        make = TRUE;
                    }
                }
            }
            /* PrintScreen 被释放 */
            if (scanCode == 0xB7) {
                if (get_byte_from_kb_buf() == 0xE0) {
                    if (get_byte_from_kb_buf() == 0xAA) {
                        key = PRINTSCREEN;
                        make = FALSE;
                    }
                }
            }

            if (key == 0) codeWithE0 = TRUE;
        }
        /* 如果不是 PrintScreen。则此时 scanCode 为 0xE0 紧跟的那个值。 */
        if ((key != PAUSEBREAK) && (key != PRINTSCREEN)) {
            /* 首先判断Make Code 还是 Break Code */
            make = (scanCode & FLAG_BREAK ? FALSE : TRUE);

            /* 先定位到 keymap 中的行 */
            keyrow = &keyMap[(scanCode & 0x7F) * MAP_COLS];

            column = 0;

            bool_t caps = shiftL || shiftR;
            if (capsLock) {
                if ((keyrow[0] >= 'a') && (keyrow[0] <= 'z')) {
                    caps = !caps;
                }
            }
            if (caps) {
                column = 1;
            }

            if (codeWithE0) {
                column = 2;
                codeWithE0 = FALSE;
            }

            key = keyrow[column];

            switch (key) {
                case SHIFT_L:
                    shiftL = make;
                    break;
                case SHIFT_R:
                    shiftR = make;
                    break;
                case CTRL_L:
                    ctrlL = make;
                    break;
                case CTRL_R:
                    ctrlR = make;
                    break;
                case ALT_L:
                    altL = make;
                    break;
                case ALT_R:
                    altR = make;
                    break;
                case CAPS_LOCK:
                    if (make) {
                        capsLock = !capsLock;
                        set_leds();
                    }
                    break;
                case NUM_LOCK:
                    if (make) {
                        numLock = !numLock;
                        set_leds();
                    }
                    break;
                case SCROLL_LOCK:
                    if (make) {
                        scrollLock = !scrollLock;
                        set_leds();
                    }
                    break;
                default:
                    break;
            }
        }

        if (make) { /* 忽略 Break Code */

            bool_t pad = FALSE;

            /* 首先处理小键盘 */
            if ((key >= PAD_SLASH) && (key <= PAD_9)) {
                pad = TRUE;
                switch (key) {    /* '/', '*', '-', '+', and 'Enter' in num pad  */
                    case PAD_SLASH:
                        key = '/';
                        break;
                    case PAD_STAR:
                        key = '*';
                        break;
                    case PAD_MINUS:
                        key = '-';
                        break;
                    case PAD_PLUS:
                        key = '+';
                        break;
                    case PAD_ENTER:
                        key = ENTER;
                        break;
                    default:    /* keys whose value depends on the NumLock */
                        if (numLock) {    /* '0' ~ '9' and '.' in num pad */
                            if ((key >= PAD_0) && (key <= PAD_9)) {
                                key = key - PAD_0 + '0';
                            } else if (key == PAD_DOT) {
                                key = '.';
                            }
                        } else {
                            switch (key) {
                                case PAD_HOME:
                                    key = HOME;
                                    break;
                                case PAD_END:
                                    key = END;
                                    break;
                                case PAD_PAGEUP:
                                    key = PAGEUP;
                                    break;
                                case PAD_PAGEDOWN:
                                    key = PAGEDOWN;
                                    break;
                                case PAD_INS:
                                    key = INSERT;
                                    break;
                                case PAD_UP:
                                    key = UP;
                                    break;
                                case PAD_DOWN:
                                    key = DOWN;
                                    break;
                                case PAD_LEFT:
                                    key = LEFT;
                                    break;
                                case PAD_RIGHT:
                                    key = RIGHT;
                                    break;
                                case PAD_DOT:
                                    key = DELETE;
                                    break;
                                default:
                                    break;
                            }
                        }
                        break;
                }
            }

            key |= shiftL ? FLAG_SHIFT_L : 0;
            key |= shiftR ? FLAG_SHIFT_R : 0;
            key |= ctrlL ? FLAG_CTRL_L : 0;
            key |= ctrlR ? FLAG_CTRL_R : 0;
            key |= altL ? FLAG_ALT_L : 0;
            key |= altR ? FLAG_ALT_R : 0;

            in_process(key, p_tty);
        }
    }
}

/* 从键盘缓冲区中读取下一个字节 */
PRIVATE u8_t get_byte_from_kb_buf() {
    u8_t scanCode;

    while (kbInPut.count <= 0) {}    /* 等待下一个字节到来 */

    interrupt_lock();
    scanCode = *(kbInPut.p_tail);
    kbInPut.p_tail++;
    if (kbInPut.p_tail == kbInPut.buf + KB_IN_BYTES) {
        kbInPut.p_tail = kbInPut.buf;
    }
    kbInPut.count--;
    interrupt_unlock();

    return scanCode;
}

/* 等待 8042 的输入缓冲区空 */
PRIVATE void kb_wait() {
    u8_t kbStat;
    do {
        kbStat = in_byte(KB_CMD);
    } while (kbStat & 0x02);
}

PRIVATE void kb_ack() {
    u8_t kbRead;

    do {
        kbRead = in_byte(KB_DATA);
    } while (kbRead != KB_ACK);
}


PRIVATE void set_leds() {
    u8_t leds = (capsLock << 2) | (numLock << 1) | scrollLock;

    kb_wait();
    out_byte(KB_DATA, LED_CODE);
    kb_ack();

    kb_wait();
    out_byte(KB_DATA, leds);
    kb_ack();
}
