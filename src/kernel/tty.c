//
// Created by 杜科 on 2020/12/8.
//

#include "core/kernel.h"
#include "core/keyboard.h"

/* 本文件内函数声明 */
PRIVATE void	init_tty(TTY* p_tty);
PRIVATE void	tty_do_read(TTY* p_tty);
PRIVATE void	tty_do_write(TTY* p_tty);
PRIVATE void put_key(TTY* p_tty, u32_t key);
PRIVATE Message msg;

PUBLIC void tty_task()
{

    TTY*	p_tty;

    init_keyboard();

    /* 初始化收发件箱 */
    io_box(&msg);

    for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
        init_tty(p_tty);
    }

    select_console(0);

    while (1) {
        if(kb_in_count()==0) {
            rec(ANY);
        }
        for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
            tty_do_read(p_tty);
            tty_do_write(p_tty);
        }

    }
}

PRIVATE void init_tty(TTY* p_tty)
{
    p_tty->inbuf_count = 0;
    p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

    init_screen(p_tty);
}


/*======================================================================*
                           in_process
 *======================================================================*/
PUBLIC void in_process(u32_t key,TTY* p_tty)
{
    if (!(key & FLAG_EXT)) {
        put_key(p_tty,key);
    }else {
        int raw_code = key & MASK_RAW;
        switch(raw_code) {
            case ENTER:
                put_key(p_tty, '\n');
                break;
            case BACKSPACE:
                put_key(p_tty, '\b');
                break;
            case UP:
                if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {	/* Shift + Up 上滚5行 */
                    scroll_screen(p_tty->p_console, SCROLL_SCREEN_UP);
                }
                break;
            case DOWN:
                if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {	/* Shift + Down */
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
                if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {	/* Alt + F1~F12 */
                    select_console(raw_code - F1);
                }
                break;
            default:
                break;
        }
    }

}

PUBLIC bool_t is_cur_console(CONSOLE* p_con){
    return (p_con == &console_table[nr_current_console]);
}

PRIVATE void tty_do_read(TTY* p_tty){
    if (is_cur_console(p_tty->p_console)) keyboard_read(p_tty);
}

PRIVATE void tty_do_write(TTY* p_tty)
{
    if (p_tty->inbuf_count) {
        char ch = *(p_tty->p_inbuf_tail);
        p_tty->p_inbuf_tail++;
        if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
            p_tty->p_inbuf_tail = p_tty->in_buf;
        }
        p_tty->inbuf_count--;

        //disp_int(ch);
        out_char(p_tty->p_console, ch);
    }
}


PRIVATE void put_key(TTY* p_tty, u32_t key)
{
    if (p_tty->inbuf_count < TTY_IN_BYTES) {
        *(p_tty->p_inbuf_head) = key;
        p_tty->p_inbuf_head++;
        if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
            p_tty->p_inbuf_head = p_tty->in_buf;
        }
        p_tty->inbuf_count++;
    }
}