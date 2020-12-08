//
// Created by 杜科 on 2020/12/8.
//

#include "core/kernel.h"
#include "core/keyboard.h"

/* 本文件内函数声明 */
PRIVATE void	init_tty(TTY* p_tty);
PRIVATE void	tty_do_read(TTY* p_tty);
PRIVATE void	tty_do_write(TTY* p_tty);


PUBLIC void tty_task()
{

    TTY*	p_tty;

    init_keyboard();

    for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
        init_tty(p_tty);
    }

    nr_current_console=0;

    while (1) {
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

    int nr_tty=p_tty-tty_table;
    p_tty->p_console=console_table+nr_tty;
}


/*======================================================================*
                           in_process
 *======================================================================*/
PUBLIC void in_process(u32_t key,TTY* p_tty)
{
    char output[2]={'\0','\0'};
    if (!(key & FLAG_EXT)) {
        if(p_tty->inbuf_count< TTY_IN_BYTES){
            *(p_tty->p_inbuf_head) =key;
            p_tty->p_inbuf_head++;
            if(p_tty->p_inbuf_head == p_tty->in_buf+TTY_IN_BYTES){
                p_tty->p_inbuf_head=p_tty->in_buf;
            }
            p_tty->inbuf_count++;
        }
    }else {
        int raw_code = key & MASK_RAW;
        switch(raw_code) {
            case UP:
                if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {	/* Shift + Up 上滚15行 */
                    interrupt_lock();
                    out_byte(CRTC_ADDR_REG,CRTC_DATA_IDX_START_ADDR_H);
                    out_byte(CRTC_DATA_REG,((8*15)>>8)&0xFF); /* 每个字符对应2字节 */
                    out_byte(CRTC_ADDR_REG,CRTC_DATA_IDX_START_ADDR_H);
                    out_byte(CRTC_DATA_REG,(8*15)&0xFF);
                    interrupt_unlock();
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
    if (is_cur_console((p_tty->p_console))) keyboard_read(p_tty);
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