//
// Created by 杜科 on 2020/12/8.
//

#ifndef AOS_CONSOLE_H
#define AOS_CONSOLE_H

/* CONSOLE */
typedef struct s_console
{
    //struct s_tty*	p_tty;
    unsigned int	current_start_addr;	/* 当前显示到了什么位置   */
    unsigned int	original_addr;		/* 当前控制台对应显存位置 */
    unsigned int	v_mem_limit;		/* 当前控制台占的显存大小 */
    unsigned int	cursor;			/* 当前光标位置 */
}CONSOLE;

#define SCROLL_SCREEN_UP	1	/* scroll forward */
#define SCROLL_SCREEN_DOWN	-1	/* scroll backward */

#define SCREEN_SIZE		(80 * 25)
#define SCREEN_WIDTH		80

#define DEFAULT_CHAR_COLOR	0x07	/* 0000 0111 黑底白字 */

#endif //AOS_CONSOLE_H
