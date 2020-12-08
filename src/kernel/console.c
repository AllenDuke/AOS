//
// Created by 杜科 on 2020/12/8.
//
#include "core/kernel.h"

PRIVATE void set_cursor(unsigned int position);

PUBLIC void out_char(CONSOLE* p_con, char ch)
{
    u8_t * p_vmem = (u8_t *)(V_MEM_BASE + g_dispPosition);

    *p_vmem++ = ch;
    *p_vmem++ =DEFAULT_CHAR_COLOR;
    g_dispPosition+=2;

    set_cursor(g_dispPosition/2);
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