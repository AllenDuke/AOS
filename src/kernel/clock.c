//
// Created by 杜科 on 2020/10/27.
//

#include "../include/core/kernel.h"

PRIVATE clock_t ticks;          /* 时钟运行的时间(滴答数)，也是开机后时钟运行的时间 */
PRIVATE Message msg;

FORWARD void clock_init (void);
FORWARD int clock_handler (int irq);

//需要设置好idt
PUBLIC void clock_task(void) {

    /* 初始化时钟 */
    clock_init();
//    interrupt_unlock();

    /* 初始化收发件箱 */
    io_box(&msg);

    printf("#{CLOCK}-> Working...\n");
    while(TRUE) {
        /* 等待外界消息 */
        rec(ANY);

        /* 为外界提供服务 */
        printf("#{CLOCK}-> get message from %d\n", msg.source);

        /* 根据处理结果，发送回复消息 */
        msg.type = 666;
        sen(msg.source);
    }
}

PRIVATE int clock_handler(int irq) {
    ticks++;
    if (ticks % 100 == 0) {
        printf(">");
//        gp_curProc++;
//        /* 超出我们的系统进程，拉回来 */
//        if (gp_curProc > proc_addr(LOW_USER)) {
//            gp_curProc = proc_addr(-NR_TASKS);
//        }
    }
    return ENABLE;  /* 返回ENABLE，使其再能发生时钟中断 */
}

PRIVATE void clock_init(void) {
    /* 设置 8253定时器芯片 的模式和计数器Counter 0以产生每秒 100 次的
     * 时钟滴答中断，即每 10ms 产生一次中断。
     */
    printf("#{clock_init}->called\n");

    /* 1 先写入我们使用的模式到 0x43 端口，即模式控制寄存器中 */
    out_byte(TIMER_MODE, RATE_GENERATOR);
    /* 2 写入计数器值的低 8 位再写入高 8 位到 Counter 0 端口 */
    out_byte(TIMER0, (u8_t) TIMER_COUNT);
    out_byte(TIMER0, (u8_t) (TIMER_COUNT >> 8));

    /* 设置时钟中断处理例程并打开其中断响应 */
    put_irq_handler(CLOCK_IRQ, clock_handler);
    enable_irq(CLOCK_IRQ);

}
