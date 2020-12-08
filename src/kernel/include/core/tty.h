//
// Created by 杜科 on 2020/12/8.
//

#ifndef AOS_TTY_H
#define AOS_TTY_H

#define TTY_IN_BYTES	256	/* tty input queue size */
/* TTY */
typedef struct s_tty
{
    u32_t in_buf[TTY_IN_BYTES];	/* TTY 输入缓冲区 */
    u32_t*	p_inbuf_head;		/* 指向缓冲区中下一个空闲位置 */
    u32_t*	p_inbuf_tail;		/* 指向键盘任务应处理的键值 */
    int	inbuf_count;		/* 缓冲区中已经填充了多少 */

    struct s_console *	p_console;
}TTY;

#endif //AOS_TTY_H