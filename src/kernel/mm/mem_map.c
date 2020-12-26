//
// Created by 杜科 on 2020/12/25.
//
#include "core/kernel.h"

PUBLIC int new_mem_map(int child_nr, int pre_nr, MemoryMap *map) {
    /* 在一个FORK调用之后，存储管理器为子进程分配内存。内核必须知道子进程位于内存何处以在运行子进程时能正确
     * 设置段寄存器。SYS_NEW_MAP消息允许存储管理器传给内核任何进程的存储映象。
     */

    register Process *proc;
    phys_addr src_phys;    /* 内存映像所在的物理地址 */
    int who;                /* 谁的内存映像？ */
    int old_flags;          /* 修改前标记的值 */

    who = child_nr;
    if (!is_ok_proc_nr(who)) return (ERROR_BAD_PROC);    /* 啊哈，这个进程索引号不正确 */
    proc = proc_addr(who);

    if (src_phys == 0) panic("错误地址\n", PANIC_ERR_NUM); /* 不可思议，MM竟然发送了一个错误的地址 */
    get_mem_map(pre_nr,map);

    /* 现在根据新的内存映像设置进程的LDT信息
     *  limit = size - 1
     */
    init_segment_desc(&proc->ldt[CS_LDT_INDEX],
                      proc->map.base,
                      (proc->map.size - 1) >> LIMIT_4K_SHIFT,
                      DA_32 | DA_LIMIT_4K | DA_C | USER_PRIVILEGE << 5
    );
    init_segment_desc(&proc->ldt[DS_LDT_INDEX],
                      proc->map.base,
                      (proc->map.size - 1) >> LIMIT_4K_SHIFT,
                      DA_32 | DA_LIMIT_4K | DA_DRW | USER_PRIVILEGE << 5
    );
//    printf("%s(nr-%d) base: %d, size: %d | ldt_sel: (c-%d|p-%d)\n", proc->name, proc->nr,
//            proc->map.base, proc->map.size, proc->ldt_sel, proc_addr(proc->nr - 1)->ldt_sel);

    old_flags = proc->flags;        /* 保存标志 */
    proc->flags &= ~NO_MAP;         /* 解开封印！将NO_MAP复位，等同于YES_MAP！！！ */
    /* 最后一步：确定旧的flags位上是否还存在除了NO_MAP以外限制进程运行的堵塞位，
     * 如果没有了，那么就可以将这个新生儿加入就绪队列了！
     */
    if (old_flags != 0 && proc->flags == 0) lock_ready(proc);
    /* Over!!! */
    return OK;
}

PUBLIC int get_mem_map(int proc_nr,MemoryMap *map){
    /* 虽然这个系统级调用是提供给所有服务器的，但是一般只有MM需要，
     * 我们报告一个进程的内存映像给调用者，它不难实现。
     */

    register Process *proc;

    if(!is_ok_proc_nr(proc_nr)) panic("进程index错误\n",PANIC_ERR_NUM);
    proc = proc_addr(proc_nr);      /* 得到进程实例，里面有我们需要的内存映像 */
    map->base=proc->map.base;
    map->size=proc->map.size;
    return OK;
}
