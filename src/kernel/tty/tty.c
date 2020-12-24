//
// Created by 杜科 on 2020/12/8.
//

#include "core/kernel.h"
#include "core/keyboard.h"

/* 本文件内函数声明 */
PRIVATE void init_tty(TTY *p_tty);

PRIVATE void tty_do_read(TTY *p_tty);

PRIVATE void tty_do_write(TTY *p_tty);

PRIVATE void put_key(TTY *p_tty, u32_t key);

PRIVATE Message msg;

PUBLIC void tty_task() {

    TTY *p_tty;

    init_keyboard();

    /* 初始化收发件箱 */
    io_box(&msg);

    for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++) {
        init_tty(p_tty);
    }

    select_console(0);

    while (1) {
        if (kb_in_count() == 0) {
//            rec(ANY);
            park();
        }
        for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++) {
            tty_do_read(p_tty);
            tty_do_write(p_tty);
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

PRIVATE void tty_do_read(TTY *p_tty) {
    if (is_cur_console(p_tty->p_console)) keyboard_read(p_tty);
}

PRIVATE char cmd[TTY_IN_BYTES];
PRIVATE u32_t cmdLen=0;
PRIVATE char* cmd_map[NR_CMDS]={
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
PRIVATE void tty_do_write(TTY *p_tty) {
    if (p_tty->inBufCount) {
        char ch = *(p_tty->p_inBufTail);
        p_tty->p_inBufTail++;
        if (p_tty->p_inBufTail == p_tty->inBuf + TTY_IN_BYTES) {
            p_tty->p_inBufTail = p_tty->inBuf;
        }
        p_tty->inBufCount--;

        //disp_int(ch);
        out_char(p_tty->p_console, ch);
        if(ch=='\n') {
            exec_cmd();
            while(cmdLen>0){
                cmd[--cmdLen]='0';
            }
            return;
        }
        cmd[cmdLen++]=ch;
    }
}
PRIVATE void exec_cmd(){
    if(cmdLen==0) return;
    int hash=0;

    /**
     * 类似java String hash值计算，不直接采取31是因为cmd一般比较短，而31又比较小，
     * 127这个质数比较大，可尽可能地使得hash的高位有值
     * hash=hash*127+cmd[i]
     */
    for(int i=0;i<cmdLen;i++){
        hash=((hash<<8)-hash)+cmd[i];
    }

    /**
     * 扰动函数
     * 同java HashMap hash()
     * hash值的高16位为原hash的高16位，低16位为原hash高16位与原hash低16位异或后的结果。
     * 原因：因为数组的长度为2的次方数，所以最终计算下标时，使用(NR_CMDS-1)&hash，
     * 这样一来，因为(NR_CMDS-1)的高位总是0，hash的高位没有参与到运算，为了进一步降低冲突，应使得高位也参与运算
     * 在权衡质量与速度后，选择了这种方式。成本不高，质量也不错。
     */
    hash=hash^(hash>>16); /* int类型 >> 符号位保持不变 */
    int index=hash&(NR_CMDS-1);

    char* pre=cmd_map[index];
    for(int i=0;i<cmdLen;i++){
        if(cmd[i]!=*pre||*pre=='\0'){
            kprintf("no such cmd:%s, hash:%d, index:%d\n",cmd,hash,index);
            return;
        }
        pre++;
    }
    //todo fork出一条进程去执行cmd
    switch (index) {
        case 0:{
            kprintf("default cmd\n");
            break;
        }
        case 1:{
            kprintf("default cmd\n");
            break;
        }
        case 2:{
            kprintf("/\n");
            break;
        }
        case 3:{
            msg.source=TTY_TASK;
            msg.type=GET_TIME;
            send_rec(CLOCK_TASK,&msg);
            kprintf("current date is: %d\n\r",msg.CLOCK_TIME);
        }
        case 4:{
            kprintf("default cmd\n");
            break;
        }
        case 5:{
            kprintf("default cmd\n");
            break;
        }
        case 6:{
            kprintf("default cmd\n");
            break;
        }
        case 7:{
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