//
// Created by 杜科 on 2020/10/27.
//

#include "../include/core/kernel.h"

PRIVATE clock_t ticks;          /* 时钟运行的时间(滴答数)，也是开机后时钟运行的时间 */
PRIVATE Message msg;

PRIVATE time_t realtime;        /* 时钟运行的时间(s)，也是开机后时钟运行的时间 */

/* 由中断处理程序更改的变量 */
PRIVATE clock_t schedule_ticks = SCHEDULE_TICKS;    /* 用户进程调度时间，当为0时候，进行程序调度 */
PRIVATE Process *last_proc;                       /* 最后使用时钟任务的用户进程 */
PRIVATE time_t boot_time;       /* 系统开机时间(s) */

PRIVATE clock_t next_alarm = ULONG_MAX;     /* 下一个闹钟发生的时刻 */
PRIVATE clock_t delay_alarm = ULONG_MAX;    /* ULONG_MAX: 毫秒级延迟函数退出；代表一个可到达的闹钟计时 */


/* 下面以年为界限，定义了每个月开始时的秒数时间数组。 */
PRIVATE int month_map[12] = {
        0,
        DAYS * (31),
        DAYS * (31 + 29),
        DAYS * (31 + 29 + 31),
        DAYS * (31 + 29 + 31 + 30),
        DAYS * (31 + 29 + 31 + 30 + 31),
        DAYS * (31 + 29 + 31 + 30 + 31 + 30),
        DAYS * (31 + 29 + 31 + 30 + 31 + 30 + 31),
        DAYS * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31),
        DAYS * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
        DAYS * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
        DAYS * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30)
};


FORWARD void clock_init (void);
FORWARD int clock_handler (int irq);
FORWARD time_t mktime(RTCTime_t *p_time);
FORWARD void do_get_uptime(void);
FORWARD void do_get_time(void);
FORWARD void do_set_time(void);
FORWARD void do_clock_int(void);

//需要设置好idt
PUBLIC void clock_task(void) {

    /* 初始化时钟 */
    clock_init();
//    interrupt_unlock();

    /* 初始化收发件箱 */
    io_box(&msg);

    /* 测试毫秒级延迟函数 */
    printf("i am zangsan, i am man!\n");
    milli_delay(sec2ms(1));
    printf("i am zangsan, no!\n");
    printf("#{CLOCK}-> Working...\n");
    while(TRUE) {
        /* 等待外界消息 */
        rec(ANY);

        /* 提供服务前，校准时间 */
        interrupt_lock();
        realtime = ticks / HZ;  /* 计算按秒算的机器真实时间 */
        interrupt_unlock();

        /* 提供服务 */
        switch (msg.type) {
            case HARD_INT:      do_clock_int();     break;
            case GET_UPTIME:    do_get_uptime();    break;
            case GET_TIME:      do_get_time();      break;
            case SET_TIME:      do_set_time();      break;
            default:    panic("#{CLOCK}-> Clock task got bad message request.\n", msg.type);
        }

        /* 根据处理结果，发送回复消息 */
        msg.type = OK;          /* 时钟驱动无可能失败的服务 */
        sen(msg.source);    /* 回复 */
    }
}

/* 10ms发生一次时钟中断 */
PRIVATE int clock_handler(int irq) {
    register Process *target;

    /* 获取当前使用时钟的进程 */
    if(kernel_reenter)  /* 发送中断重入，说明当前处于核心代码段，被中断的进程使用虚拟硬件 */
        target = proc_addr(HARDWARE);
    else                /* 正常中断，被中断的进程就是当前运行的进程 */
        target = gp_curProc;

    /* 计时 */
    ticks++;

    /* 记账：给使用了系统资源的用户进程记账 */
    target->user_time++;        /* 用户时间记账 */

    if(gp_curProc==proc_addr(IDLE_TASK)&&gp_curProc->user_time%10==0) {
        lock_hunter();
    }

    if(target != bill_proc && target != proc_addr(HARDWARE))
        bill_proc->sys_time++;  /* 当前进程不是计费的用户进程，那么它应该是使用了系统调用陷入了内核，记录它的系统时间 */

    /* 闹钟时间到了？产生一个时钟中断，唤醒时钟任务 */
    if( next_alarm <= ticks ) {
        interrupt(CLOCK_TASK);
        return ENABLE;
    }

    /* 毫秒级休眠函数退出闹钟响了？ */
    if( delay_alarm <= ticks ) {
        delay_alarm = ULONG_MAX;    /* 关闭毫秒级延迟闹钟 */
    }


    /* 重置用户进程调度时间片 */
    if(--schedule_ticks == 0) {
        schedule_ticks = SCHEDULE_TICKS;
        last_proc = bill_proc;  /* 记录最后一个消费进程 */
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
    /* 设置正确的开机启动时间 */
    RTCTime_t now;
    get_rtc_time(&now);
    boot_time = mktime(&now);
    printf("#{CLOCK}-> now is %d-%d-%d %d:%d:%d\n",
           now.year, now.month, now.day, now.hour, now.minute, now.second);
    printf("#{CLOCK}-> boot startup time is %ld\n", boot_time);
}

/*===========================================================================*
 *				do_get_uptime				     *
 *			获取时钟运行时间(tick)
 *===========================================================================*/
PRIVATE inline void do_get_uptime(void) {
    msg.CLOCK_TIME = ticks;     /* 设置到消息中，将会回复给请求者 */
}

/*===========================================================================*
 *				do_get_time				     *
 *			获取时钟实时时间(s)
 *===========================================================================*/
PRIVATE inline void do_get_time(void) {
    msg.CLOCK_TIME = (long) (boot_time + realtime);     /* 实时时间 = 开机时间 + 时钟运行时间 */
}

/*===========================================================================*
 *				do_set_time				     *
 *			设置时钟实时时间(s)
 *===========================================================================*/
PRIVATE inline void do_set_time(void) {
    boot_time = msg.CLOCK_TIME - realtime;  /* 系统启动时间 = 用户设置的时间 - 时钟运行时间 */
}


/*===========================================================================*
 *				do_get_rtc_time					     *
 *		        获取硬件系统实时时间
 *===========================================================================*/
PUBLIC void get_rtc_time(RTCTime_t *p_time) {
    /* 这个例程很简单，不断的从 CMOS 的端口中获取时间的详细数据 */
    u8_t status;

    p_time->year = cmos_read(YEAR);
    p_time->month = cmos_read(MONTH);
    p_time->day = cmos_read(DAY);
    p_time->hour = cmos_read(HOUR);
    p_time->minute = cmos_read(MINUTE);
    p_time->second = cmos_read(SECOND);

    /* 查看 CMOS 返回的 RTC 时间是不是 BCD 码？
     * 如果是，我们还需要手动将 BCD 码转换成十进制。
     */
    status = cmos_read(CLK_STATUS);
    if( (status & 0x4) == 0 ) {
        p_time->year = bcd2dec(p_time->year);
        p_time->month = bcd2dec(p_time->month);
        p_time->day = bcd2dec(p_time->day);
        p_time->hour = bcd2dec(p_time->hour);
        p_time->minute = bcd2dec(p_time->minute);
        p_time->second =bcd2dec(p_time->second);
    }
    p_time->year += 2000;   /* CMOS 记录的年是从 2000 年开始的，我们补上 */
}

/*===========================================================================*
 *				mktime				     *
 *			设置正确的开机时间
 *===========================================================================*/
PRIVATE time_t mktime(RTCTime_t *p_time) {
    /* 这个函数只供内核使用，所以我们不需要关心 1970 以前的年份。
     * 计算从 1970 年 1 月 1 日 0 时起到开机当日经过的秒数，作为开机时间。
     * 对于这个转换后的时间，即 UNIX 的传统，被称为 UNIX 时间戳。
     */

    time_t now;
    u16_t year = p_time->year;
    u8_t mouth = p_time->month;
    u16_t day = p_time->day;
    u8_t hour = p_time->hour, minute = p_time->minute, second = p_time->second;

    year -= 1970;                   /* 从 1970 开始 */
    /* 为了获得正确的闰年数，这里需要这样一个魔幻偏值 year + 1 */
    now = YEARS * year + DAYS * ( (year + 1) / 4 );     /* 这些年经过的秒数时间 + 每个闰年时多 1 天的秒数时间 */
    now += month_map[mouth - 1];                        /* 再加上当年到当月的秒数 */
    /* 如果 (year + 2) 不是闰年，我们就需要进行调整（减去一天的秒数） */
    if( mouth - 1 > 0 && ( (year + 2) % 4 ) ) {
        now -= DAYS;
    }
    now += DAYS * (day - 1);        /* 加上本月过去的天数的秒数 */
    now += HOURS * hour;            /* 加上当天过去的小时的秒数 */
    now += MINUTES * minute;        /* 加上一小时内过去的分钟的秒数 */
    now += second;                  /* 加上一分钟内过去的秒数 */
    /* 这点我们很容易忽略，因为我们要北京时间，需要减去 8 个时区 */
    return (now - (8 * HOURS));
}

PUBLIC void milli_delay(time_t delay_ms) {
    /* 毫秒级的延迟函数
      *
      * 这个函数是为需要极短延迟的任务提供的。它用 C 语言编写，没有引入任何硬件相关性，
      * 但是使用了一种人们只有在低级汇编语言中找到的技术。它把计数器初始化为零，然后
      * 对其快速轮询直到到达指定的值。
      * 这种忙等待技术一般应该避免，但是，实现的必要性要求不能遵循一般的规则。
      *
      * 注意：毫秒级，也只能达到 10 毫秒内的延迟
      * 因为 10ms 是因为我们设置系统发生时钟中断的时间间隔，我们没法做到更精确于此值的
      * 延迟，但是对于所有需要精确的延迟技术的函数，这已经足够了。
      * 本函数也能被时钟任务调用，因为一旦时钟中断打开，本函数就能正常运行了，所以必须
      * 要在中时钟中断初始化后才能被正常调用。
      */

    /* 得出退出循环的闹钟时间 */
    delay_alarm = ticks + delay_ms / ONE_TICK_MILLISECOND;
    /* 只要检测到毫秒级闹钟未被关闭，说明时候未到，继续死循环 */
    while ( delay_alarm != ULONG_MAX ) {  }
}

/*===========================================================================*
 *				do_clock_int				     *
 *			    处理时钟(软)中断
 *===========================================================================*/
PRIVATE void do_clock_int(void) {
    /* 尽管这个例程叫做这个名字，但要注意的一点是，并不是在每次时钟中断都会调用
     * 这个例程的。当中断处理程序确定有一些重要的工作不得不做时才会通知时钟任务
     * 让时钟任务来调用本例程。
     */

    printf("i am clock int, hi brother!\n");
    next_alarm = ULONG_MAX;
}


