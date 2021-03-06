//
// Created by 杜科 on 2020/10/16.
//
/**
 * 内核所需的所有函数原型
 * 所有那些必须在其定义所在文件外被感知的函数的原型都放在prototype.h中。
 *
 * 这其中的许多函数原型是与系统相关的,包括中断和异常处理例程以及用汇编语言写的一些函数。
 */

#ifndef AOS_PROTOTYPE_H
#define AOS_PROTOTYPE_H


#include "message.h"
#include "process.h"
#include "tty.h"
#include "mm.h"
#include "console.h"

//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  init_c.c
//----------------------------------------------------------------------------------------------------------------------
void init_c(void);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  protect.c
//----------------------------------------------------------------------------------------------------------------------
void init_segment_desc(SegDescriptor *p_desc, phys_addr base,u32_t limit, u16_t attribute);
void init_protect(void);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  kernel_i386lib.asm
//----------------------------------------------------------------------------------------------------------------------
void phys_copy(phys_addr src, phys_addr dest, u32_t size);
void low_print(char* p_str);
void cpu_halt(void);
u8_t in_byte(port_t port);
void out_byte(port_t port, u8_t value);
void port_read(u16_t port, void *dest, unsigned byteCount);
void port_write(u16_t port, void *source, unsigned byteCount);
void interrupt_lock(void);
void interrupt_unlock(void);
int disable_irq(int int_request);
void enable_irq(int int_request);
void restart(void);
void aos_sys_call(void);
void msg_copy(phys_addr msg_phys, phys_addr dest_phys);
u8_t cmos_read(u8_t addr);
void level0(aos_syscall level0_func);
void halt(void);
void level0_sys_call(void);
void park_sys_call(void);
void unpark_sys_call(void);

// 异常处理例程
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

// 硬件中断处理例程
void hwint00(void);
void hwint01(void);
void hwint02(void);
void hwint03(void);
void hwint04(void);
void hwint05(void);
void hwint06(void);
void hwint07(void);
void hwint08(void);
void hwint09(void);
void hwint10(void);
void hwint11(void);
void hwint12(void);
void hwint13(void);
void hwint14(void);
void hwint15(void);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  clock.c
//----------------------------------------------------------------------------------------------------------------------
void clock_task(void);
void get_rtc_time(struct rtc_time *p_time);
void milli_delay(time_t delay_ms);
clock_t clock_get_uptime();
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  i8259.c
//----------------------------------------------------------------------------------------------------------------------
void init_8259A(void);
void put_irq_handler(int irq, irq_handler handler);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  panic.c
//----------------------------------------------------------------------------------------------------------------------
void panic(const char* p_msg, int errorNum );
void bad_assertion(char *p_file, int line, char *p_srcCode);
void bad_compare(char *p_file, int line, int lhs, char *p_srcCode, int rhs);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  lib/stdio/printf.c
//----------------------------------------------------------------------------------------------------------------------
int fmt_string(char *p_buf, const char *p_fmt, ...);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  process.c
//----------------------------------------------------------------------------------------------------------------------
void lock_schedule(void);
void lock_unready(struct process_s *p_proc);
void lock_ready(struct process_s *p_proc);
void lock_hunter(void);
void schedule_stop(void );
void ready(struct process_s *p_proc);
void unready(struct process_s *p_proc);
void interrupt(int task);
void unhold(void);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  ipc.c
//----------------------------------------------------------------------------------------------------------------------
//int sys_call(int op, int src_dest_msgp, Message *msg_ptr); //不向外暴露
int aos_send(struct process_s *p_caller, int dest, Message *p_msg);
int aos_receive(struct process_s *p_caller, int src, Message *p_msg);
void aos_park();
void aos_unpark(int pid);
void rm_proc_from_waiters(Process* proc);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  dump.c
//----------------------------------------------------------------------------------------------------------------------
void proc_dump(void);
void map_dump(void);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  exception.c
//----------------------------------------------------------------------------------------------------------------------
//void exception_handler(int int_vector, int error_no); //不向外暴露
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  console.c
//----------------------------------------------------------------------------------------------------------------------
void out_char(CONSOLE* p_con, char ch);
void init_screen(TTY* p_tty);
void select_console(int console_num);
void scroll_screen(CONSOLE* p_con, int direction);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  keyboard.c
//----------------------------------------------------------------------------------------------------------------------
int keyboard_handler(int irq);
void keyboard_read(TTY* p_tty);
void init_keyboard();
int kb_in_count();
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  idle.c
//----------------------------------------------------------------------------------------------------------------------
void idle_task(void);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  tty.c
//----------------------------------------------------------------------------------------------------------------------
bool_t is_cur_console(CONSOLE* p_con);
void in_process(u32_t key,TTY* p_tty);
void tty_task();
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  mm.c
//----------------------------------------------------------------------------------------------------------------------
void mm_task(void);
void set_reply(int proc_nr,int rs);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  alloc.c
//----------------------------------------------------------------------------------------------------------------------
void mem_init(phys_page base, phys_page freePages);
phys_page alloc(phys_page applyPages);
void free(phys_page begin,phys_page size);
void mem_dump();
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  mem_map.c
//----------------------------------------------------------------------------------------------------------------------
int new_mem_map(int child_nr, int pre_nr, MemoryMap *map);
int get_mem_map(int proc_nr,MemoryMap *map);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  fork.c
//----------------------------------------------------------------------------------------------------------------------
int fork(void);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  exit.c
//----------------------------------------------------------------------------------------------------------------------
void exit_cleanup(register MMProcess *exit_proc,MMProcess *wait_parent);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  wait.c
//----------------------------------------------------------------------------------------------------------------------
int wait(void);
int waitpid(void);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  at_wini.c
//----------------------------------------------------------------------------------------------------------------------
void at_winchester_task(void);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  fs.c
//----------------------------------------------------------------------------------------------------------------------
void fs_task(void);
struct super_block *get_super_block(int dev);
int rw_sector(int io_type, int dev, u64_t pos, int bytes, int proc_nr,void *buf);
struct inode *get_inode(int dev, int num);
void put_inode(struct inode *pinode);
void sync_inode(struct inode *p);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  open.c
//----------------------------------------------------------------------------------------------------------------------
int do_open();
int do_close();
int do_lseek();
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  fs_misc.c
//----------------------------------------------------------------------------------------------------------------------
int do_stat();
int search_file(char * path);
int strip_path(char * filename, const char * pathname,struct inode** ppinode);
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  read_write.c
//----------------------------------------------------------------------------------------------------------------------
int do_rdwt();
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  link.c
//----------------------------------------------------------------------------------------------------------------------
int do_unlink();
//======================================================================================================================


//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
//  test.c
//----------------------------------------------------------------------------------------------------------------------
void fs_test();
void tty_test();
//======================================================================================================================


#endif //AOS_PROTOTYPE_H
