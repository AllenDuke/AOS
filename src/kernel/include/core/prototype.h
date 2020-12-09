//
// Created by 杜科 on 2020/10/16.
//
/**
 * 内核所需的所有函数原型
 * 所有那些必须在其定义所在文件外被感知的函数的原型都放在prototype.h中。
 *
 * 它使用了_PROTOTYPE技术，这样，AOS核心便既可以使用传统的C编译器(由Kernighan和Richie定义)，
 * 例如Minix初始提供的编译器；又可以使用一个现代的ANSI标准C编译器。
 *
 * 这其中的许多函数原型是与系统相关的,包括中断和异常处理例程以及用汇编语言写的一些函数。
 */

#ifndef AOS_PROTOTYPE_H
#define AOS_PROTOTYPE_H

/*================================================================================================*/
/* init_c.c */
/*================================================================================================*/
void init_c(void);

/*================================================================================================*/
/* protect.c */
/*================================================================================================*/
void init_segment_desc(SegDescriptor *p_desc, phys_addr base,u32_t limit, u16_t attribute);
void init_protect(void);

/*================================================================================================*/
/* kernel_i386lib.asm */
/*================================================================================================*/
void phys_copy(phys_addr src, phys_addr dest, u32_t size);
void low_print(char* _str);
void cpu_halt(void);
u8_t in_byte(port_t port);
void out_byte(port_t port, u8_t value);
u16_t in_word(port_t port);
void out_word(port_t port, u16_t value);
void interrupt_lock(void);
void interrupt_unlock(void);
int disable_irq(int int_request);
void enable_irq(int int_request);
void restart(void);
void aos_sys_call(void);
void msg_copy(phys_addr msg_phys, phys_addr dest_phys);
u8_t cmos_read(u8_t addr);
/*================================================================================================*/
/* clock.c */
/*================================================================================================*/
void clock_task(void);
void get_rtc_time(struct rtc_time *p_time);
void milli_delay(time_t delay_ms);

/* 硬件（异常）中断处理函数原型 相当于声明函数指针 int_handler_t 不能如此声明 void int_handler_t (void) */
typedef void (*int_handler)(void);
/* 中断请求处理函数原型 */
typedef int (*irq_handler)(int irq);
/* 系统调用函数原型 */
typedef void (*aos_syscall)(void);

/*================================================================================================*/
/* i8259.c */
/*================================================================================================*/
void init_8259A(void);
void put_irq_handler(int irq, irq_handler handler);

/*================================================================================================*/
/* 异常处理入口例程 */
/*================================================================================================*/
void divide_error(void);
void debug_exception(void);
void non_maskable_int(void);
void break_point(void);
void over_flow(void);
void out_of_bounds(void);
void invalid_opcode(void);
void dev_not_available(void);
void double_fault(void);
void coop_proc_seg_oob(void);
void invalid_tss(void);
void segment_not_present(void);
void stack_exception(void);
void general_protection(void);
void page_fault(void);
void math_fault(void);

/*================================================================================================*/
/*  硬件中断处理程序。 */
/*================================================================================================*/
void    hwint00(void);
void	hwint01(void);
void	hwint02(void);
void	hwint03(void);
void	hwint04(void);
void	hwint05(void);
void	hwint06(void);
void	hwint07(void);
void	hwint08(void);
void	hwint09(void);
void	hwint10(void);
void	hwint11(void);
void	hwint12(void);
void	hwint13(void);
void	hwint14(void);
void	hwint15(void);


void panic(const char* msg, int error_no );
void test_task_a(void);
void test_task_b(void);

int printf(const char *_fmt, ...);
int fmt_string(char *_buf, const char *_fmt, ...);

void idle_task(void);
void level0(aos_syscall level0_func);
void halt(void);
void level0_sys_call(void);

/* 公有函数声明 */
/*================================================================================================*/
/* process.c */
/*================================================================================================*/
void lock_schedule(void);
void lock_unready(struct process_s *proc);
void lock_ready(struct process_s *proc);
void lock_hunter(void);
void schedule_stop(void );
void ready(struct process_s *proc);
void unready(struct process_s *proc);
void interrupt(int task);
void unhold(void);

/*================================================================================================*/
/* ipc_msg.c */
/*================================================================================================*/
int flyanx_send(struct process_s *caller, int dest, Message *msg_phys);
int flyanx_receive(struct process_s *caller, int src, Message *msg_phys);

/*================================================================================================*/
/* dump.c */
/*================================================================================================*/
void proc_dump(void);
void map_dump(void);

int keyboard_handler(int irq);
void keyboard_read(TTY* p_tty);
void in_process(u32_t key,TTY* p_tty);
void init_keyboard();
void tty_task();
void out_char(CONSOLE* p_con, char ch);

void init_screen(TTY* p_tty);
void select_console(int nr_console);
void scroll_screen(CONSOLE* p_con, int direction);
#endif //AOS_PROTOTYPE_H
