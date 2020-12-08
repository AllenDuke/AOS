//
// Created by 杜科 on 2020/12/8.
//

#include "core/kernel.h"
#include "core/keyboard.h"
#include "core/keymap.h"

PRIVATE	KB_INPUT	kb_in;

PRIVATE	bool_t		code_with_E0	= FALSE;
PRIVATE	bool_t		shift_l;		/* l shift state	*/
PRIVATE	bool_t		shift_r;		/* r shift state	*/
PRIVATE	bool_t		alt_l;			/* l alt state		*/
PRIVATE	bool_t		alt_r;			/* r left state		*/
PRIVATE	bool_t		ctrl_l;			/* l ctrl state		*/
PRIVATE	bool_t		ctrl_r;			/* l ctrl state		*/
PRIVATE	int		column		= 0;	/* keyrow[column] 将是 keymap 中某一个值 */

/* 本文件内函数声明 */
PRIVATE u8_t 	get_byte_from_kb_buf();


/*======================================================================*
                            keyboard_handler
 *======================================================================*/
PUBLIC int keyboard_handler(int irq)
{
    u8_t scan_code = in_byte(KB_DATA);

    if (kb_in.count < KB_IN_BYTES) {
        *(kb_in.p_head) = scan_code;
        kb_in.p_head++;
        if (kb_in.p_head == kb_in.buf + KB_IN_BYTES) {
            kb_in.p_head = kb_in.buf;
        }
        kb_in.count++;
    }
    
    return ENABLE;
}


/*======================================================================*
                           init_keyboard
 *======================================================================*/
PUBLIC void init_keyboard()
{
    kb_in.count = 0;
    kb_in.p_head = kb_in.p_tail = kb_in.buf;

    put_irq_handler(KEYBOARD_IRQ, keyboard_handler);	/* 设定键盘中断处理程序 */
    enable_irq(KEYBOARD_IRQ);				/* 开键盘中断 */
}

PUBLIC void keyboard_read(TTY* p_tty){
    u8_t	scan_code;
    char output[2];
    bool_t	make;	/* TRUE : make  */
    /* FALSE: break */
    u32_t	key = 0;/* 用一个整型来表示一个键。 */
    /* 比如，如果 Home 被按下，则 key 值将为定义在 keyboard.h 中的 'HOME'。*/
    u32_t*	keyrow;	/* 指向 keymap[] 的某一行 */

    if(kb_in.count > 0){
        code_with_E0 = FALSE;
        scan_code = get_byte_from_kb_buf();

        /* 下面开始解析扫描码 */
        if (scan_code == 0xE1) {
            int i;
            u8_t pausebreak_scan_code[] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};
            bool_t is_pausebreak = TRUE;
            for(i=1;i<6;i++){
                if (get_byte_from_kb_buf() != pausebreak_scan_code[i]) {
                    is_pausebreak = FALSE;
                    break;
                }
            }
            if (is_pausebreak) {
                key = PAUSEBREAK;
            }
        }else if (scan_code == 0xE0) {

            scan_code = get_byte_from_kb_buf();

            /* PrintScreen 被按下 */
            if (scan_code == 0x2A) {
                if (get_byte_from_kb_buf() == 0xE0) {
                    if (get_byte_from_kb_buf() == 0x37) {
                        key = PRINTSCREEN;
                        make = TRUE;
                    }
                }
            }
                /* PrintScreen 被释放 */
            if (scan_code == 0xB7) {
                if (get_byte_from_kb_buf() == 0xE0) {
                    if (get_byte_from_kb_buf() == 0xAA) {
                        key = PRINTSCREEN;
                        make = FALSE;
                    }
                }
            }

            if(key==0) code_with_E0=TRUE;
        }
        /* 如果不是 PrintScreen。则此时 scan_code 为 0xE0 紧跟的那个值。 */
        if ((key != PAUSEBREAK) && (key != PRINTSCREEN)) {
            /* 首先判断Make Code 还是 Break Code */
            make = (scan_code & FLAG_BREAK ? FALSE : TRUE);

            /* 先定位到 keymap 中的行 */
            keyrow = &keymap[(scan_code & 0x7F) * MAP_COLS];

            column = 0;

            bool_t caps = shift_l || shift_r;

            if (caps) {
                column = 1;
            }

            if (code_with_E0) {
                column = 2;
                code_with_E0=FALSE;
            }

            key = keyrow[column];

            switch(key) {
                case SHIFT_L:
                    shift_l	= make;
                    break;
                case SHIFT_R:
                    shift_r	= make;
                    break;
                case CTRL_L:
                    ctrl_l	= make;
                    break;
                case CTRL_R:
                    ctrl_r	= make;
                    break;
                case ALT_L:
                    alt_l	= make;
                    break;
                case ALT_R:
                    alt_r	= make;
                    break;
                default:
                    break;
            }
        }

        if(make){ /* 忽略 Break Code */
            key |= shift_l	? FLAG_SHIFT_L	: 0;
            key |= shift_r	? FLAG_SHIFT_R	: 0;
            key |= ctrl_l	? FLAG_CTRL_L	: 0;
            key |= ctrl_r	? FLAG_CTRL_R	: 0;
            key |= alt_l	? FLAG_ALT_L	: 0;
            key |= alt_r	? FLAG_ALT_R	: 0;

            in_process(key,p_tty);
        }
    }
}

PRIVATE u8_t get_byte_from_kb_buf()	/* 从键盘缓冲区中读取下一个字节 */
{
    u8_t	scan_code;

    while (kb_in.count <= 0) {}	/* 等待下一个字节到来 */

    interrupt_lock();
    scan_code = *(kb_in.p_tail);
    kb_in.p_tail++;
    if (kb_in.p_tail == kb_in.buf + KB_IN_BYTES) {
        kb_in.p_tail = kb_in.buf;
    }
    kb_in.count--;
    interrupt_unlock();

    return scan_code;
}