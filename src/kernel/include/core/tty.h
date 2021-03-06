//
// Created by 杜科 on 2020/12/8.
//

#ifndef AOS_TTY_H
#define AOS_TTY_H

#define TTY_IN_BYTES    256    /* tty input queue size */
#define TTY_OUT_BUF_LEN		2	/* tty output buffer size */
/* TTY */
typedef struct tty_s {
    u32_t inBuf[TTY_IN_BYTES];      /* TTY 输入缓冲区 */
    u32_t *p_inBufHead;             /* 指向缓冲区中下一个空闲位置 */
    u32_t *p_inBufTail;             /* 指向键盘任务应处理的键值 */
    int inBufCount;             /* 缓冲区中已经填充了多少 */

    int	tty_caller;
    int	tty_procnr;
    void*	tty_req_buf;
    int	tty_left_cnt;
    int	tty_trans_cnt;

    struct s_console *p_console;
} TTY;

#define TTY_FIRST	(ttys)
#define TTY_END		(ttys+ NR_CONSOLES)

#endif //AOS_TTY_H
