//
// Created by 杜科 on 2020/12/8.
//
#include "core/kernel.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32_t addr);

PUBLIC void out_char(CONSOLE* p_con, char ch)
{
    u8_t * p_vmem = (u8_t *)(V_MEM_BASE + p_con->cursor*2);

    *p_vmem++ = ch;
    *p_vmem++ =DEFAULT_CHAR_COLOR;
    p_con->cursor++;

    set_cursor(p_con->cursor);
}

PRIVATE void set_cursor(unsigned int position)
{
    interrupt_lock();
    out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_CURSOR_H);
    out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_CURSOR_L);
    out_byte(CRTC_DATA_REG, position & 0xFF);
    interrupt_unlock();
}

PUBLIC void init_screen(TTY* p_tty)
{
    int nr_tty = p_tty - tty_table;
    p_tty->p_console = console_table + nr_tty;

    int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */

    int con_v_mem_size			= v_mem_size / NR_CONSOLES;		/* 每个控制台占的显存大小		(in WORD) */
    p_tty->p_console->original_addr		= nr_tty * con_v_mem_size;		/* 当前控制台占的显存开始地址		(in WORD) */
    p_tty->p_console->v_mem_limit		= con_v_mem_size;			/* 当前控制台占的显存大小		(in WORD) */
    p_tty->p_console->current_start_addr	= p_tty->p_console->original_addr;	/* 当前控制台显示到了显存的什么位置	(in WORD) */

    p_tty->p_console->cursor = p_tty->p_console->original_addr;	/* 默认光标位置在最开始处 */

    if (nr_tty == 0) {
        p_tty->p_console->cursor = g_dispPosition / 2;	/* 第一个控制台延用原来的光标位置 */
    }
    else {
        out_char(p_tty->p_console, nr_tty + '0');
        out_char(p_tty->p_console, '#');
    }

    set_cursor(p_tty->p_console->cursor);
}

PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
    if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {	/* invalid console number */
        return;
    }

    nr_current_console = nr_console;

    set_cursor(console_table[nr_console].cursor);
    set_video_start_addr(console_table[nr_console].current_start_addr);
}

PRIVATE void set_video_start_addr(u32_t addr)
{
    interrupt_lock();
    out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_START_ADDR_H);
    out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_START_ADDR_L);
    out_byte(CRTC_DATA_REG, addr & 0xFF);
    interrupt_unlock();
}
/*======================================================================*
                           scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCROLL_SCREEN_UP	: 向上滚屏
	SCROLL_SCREEN_DOWN	: 向下滚屏
	其它			: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
    if (direction == SCROLL_SCREEN_UP) {
        if (p_con->current_start_addr > p_con->original_addr) {
            p_con->current_start_addr -= SCREEN_WIDTH;
        }
    }
    else if (direction == SCROLL_SCREEN_DOWN) {
        if (p_con->current_start_addr + SCREEN_SIZE < p_con->original_addr + p_con->v_mem_limit) {
            p_con->current_start_addr += SCREEN_WIDTH;
        }
    }
    else{
    }

    set_video_start_addr(p_con->current_start_addr);
    set_cursor(p_con->cursor);
}
