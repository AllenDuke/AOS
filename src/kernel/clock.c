//
// Created by 杜科 on 2020/10/27.
//

#include "../include/core/kernel.h"

FORWARD _PROTOTYPE( void clock_init, (void) );
FORWARD _PROTOTYPE( int clock_handler, (int irq) );

//需要设置好idt
PUBLIC void clock_task(void){

    /* 初始化时钟 */
    clock_init();
    interrupt_unlock();

}

PRIVATE void clock_init(void) {
    /* 设置 8253定时器芯片 的模式和计数器Counter 0以产生每秒 100 次的
     * 时钟滴答中断，即每 10ms 产生一次中断。
     */

    /* 1 先写入我们使用的模式到 0x43 端口，即模式控制寄存器中 */
    out_byte(TIMER_MODE, RATE_GENERATOR);
    /* 2 写入计数器值的低 8 位再写入高 8 位到 Counter 0 端口 */
    out_byte(TIMER0, (u8_t)TIMER_COUNT);
    out_byte(TIMER0, (u8_t)(TIMER_COUNT >> 8));

    /* 设置时钟中断处理例程并打开其中断响应 */
    put_irq_handler(CLOCK_IRQ, clock_handler);
    enable_irq(CLOCK_IRQ);

}

PRIVATE int clock_handler(int irq) {
    printf(">");
    return ENABLE;  /* 返回ENABLE，使其再能发生时钟中断 */
}