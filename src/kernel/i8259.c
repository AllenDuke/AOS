//
// Created by 杜科 on 2020/10/24.
//

#include "../include/core/kernel.h"


/* 本地函数声明 */
FORWARD int default_irq_handler (int irq);

/* 初始化中断 */
PUBLIC void init_8259A(){

    /* 初始化前先将中断响应关闭 */
//    interrupt_lock();

    /**
     * 这里的具体意思参考，《自己动手写操作系统》P112
     */
    /* 1  向端口20H(主片)和a0H(从片)写入ICW1 */
    out_byte(INT_M_CTL, 0x11);
    out_byte(INT_S_CTL, 0x11);
    /* 2 向端口21H(主片)和a1H(从片)写入ICW2 */
    out_byte(INT_M_CTL_MASK, INT_VECTOR_IRQ0); /* master对应中断向量32~39 */
    out_byte(INT_S_CTL_MASK, INT_VECTOR_IRQ8); /* slave对应中断向量40~47 */
    /* 3 向端口21H(主片)或a1H(从片)写入ICW3 */
    out_byte(INT_M_CTL_MASK, 4);
    out_byte(INT_S_CTL_MASK, 2);
    /* 4 向端口21H(主片)或a1H(从片)写入ICW4 */
    out_byte(INT_M_CTL_MASK, 1);
    out_byte(INT_S_CTL_MASK, 1);

    /* 由于现在还没有配置中断例程，我们屏蔽所有中断，使其都不能发生 */
    out_byte(INT_M_CTL_MASK, 0xff);
    out_byte(INT_S_CTL_MASK, 0xff);

    /* 最后，我们初始化中断处理程序表，给每一个中断设置一个默认的中断处理例程 */
    for(int i = 0; i < NR_IRQ_VECTORS; i++) {
        g_irqHandlers[i] = default_irq_handler;
    }

}

/**
 * 设置并注册中断处理例程
 * @param irq
 * @param handler
 */
PUBLIC void put_irq_handler(int irq, irq_handler handler){
    /**
     * 一旦一个硬件的驱动编写完成，那么就可以调用本例程来
     * 为其设置真正的中断处理例程了，它将会替换掉初始化时
     * 默认的spurious_irq例程。
     */

    /* 断言：中断向量处于正常范围 */
//    assert(irq >= 0 && irq < NR_IRQ_VECTORS);

    /* 注册过了？那么什么也不做 */
    if(g_irqHandlers[irq] == handler) return;

    /* 断言：该中断已初始化过 */
//    assert(irq_handler_table[irq] == default_irq_handler);

    /* 开始设置
     * 先关闭对应的中断，再将中断处理例程替换旧的
     */
    disable_irq(irq);
    g_irqHandlers[irq] = handler;
}

/**
 * 默认的中断处理例程
 * 响应后会自动关闭中断
 * @param irq
 * @return 0 响应完毕不需要重新打开中断
 */
PRIVATE int default_irq_handler(int irq){
    kprintf("I am a interrupt, my name is int %d\n", irq);
    return DISABLE; /* 如果是键盘这样的中断，需要持续地响应，即响应完毕后要重新打开中断，这样的话要返回非0 */
}