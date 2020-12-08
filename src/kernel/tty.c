//
// Created by 杜科 on 2020/12/8.
//

#include "core/kernel.h"
#include "keyboard.h"

PUBLIC void tty_task()
{

    while (1) {
        keyboard_read();
    }
}

/*======================================================================*
                           in_process
 *======================================================================*/
PUBLIC void in_process(u32_t key)
{
    char output[2]={'\0','\0'};
    if (!(key & FLAG_EXT)) {
        output[0]=key&0XFF;
        low_print(output); //todo 这里如果用printf会触发GP异常
    }

//    if (!(key & FLAG_EXT)) {
//        output[0]=key&0xFF;
//        printf("%s\n",output);
//
//        interrupt_lock();
//        out_byte(CRTC_ADDR_REG,CRTC_DATA_IDX_CURSOR_H);
//        out_byte(CRTC_DATA_REG,((g_dispPosition/2)>>8)&0xFF); /* 每个字符对应2字节 */
//        out_byte(CRTC_ADDR_REG,CRTC_DATA_IDX_CURSOR_L);
//        out_byte(CRTC_DATA_REG,(g_dispPosition/2)&0xFF);
//        interrupt_unlock();
//    }else {
//        int raw_code = key & MASK_RAW;
//        switch(raw_code) {
//            case UP:
//                if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {	/* Shift + Up */
//                    interrupt_lock();
//                    out_byte(CRTC_ADDR_REG,CRTC_DATA_IDX_CURSOR_H);
//                    out_byte(CRTC_DATA_REG,((8*15)>>8)&0xFF); /* 每个字符对应2字节 */
//                    out_byte(CRTC_ADDR_REG,CRTC_DATA_IDX_CURSOR_L);
//                    out_byte(CRTC_DATA_REG,(8*15)&0xFF);
//                    interrupt_unlock();
//                }
//                break;
//            default:
//                break;
//        }
//    }

}
