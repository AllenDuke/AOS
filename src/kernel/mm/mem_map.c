//
// Created by 杜科 on 2020/12/25.
//
#include "core/kernel.h"

extern MMProcess mmProcs[];

/**
 *
 * @param child_nr 子进程逻辑索引号
 * @param pre_nr 父进程逻辑索引号
 * @param map 子进程MemoryMap
 * @return
 */
PUBLIC int new_mem_map(int child_nr, int pre_nr) {

    register Process *proc=proc_addr(child_nr);
    int old_flags;          /* 修改前标记的值 */

    if (!is_ok_proc_nr(child_nr)) return (ERROR_BAD_PROC);    /* 啊哈，这个进程索引号不正确 */

//    get_mem_map(pre_nr,&(proc->map)); /* 复制父进程的MemoryMap */
    proc->map.base=mmProcs[child_nr].map.base;
    proc->map.size=mmProcs[child_nr].map.size;

    /**
     * 现在根据新的内存映像设置进程的LDT信息
     *  limit = size - 1
     */
    init_segment_desc(&proc->ldt[CS_LDT_INDEX],
                      proc->map.base,
                      (proc->map.size - 1) >> LIMIT_4K_SHIFT,
                      DA_32 | DA_LIMIT_4K | DA_C | USER_PRIVILEGE << 5
    );
    init_segment_desc(&proc->ldt[DS_LDT_INDEX],
                      proc->map.base,
                      (proc->map.size - 1) >> LIMIT_4K_SHIFT,   //todo 让它可以访问全部空间
                      DA_32 | DA_LIMIT_4K | DA_DRW | USER_PRIVILEGE << 5
    );

    old_flags = proc->flags;        /* 保存标志 */
    proc->flags &= ~NO_MAP;         /* 解开封印！将NO_MAP复位，等同于YES_MAP！！！ */

    /**
     * 最后一步：确定旧的flags位上是否还存在除了NO_MAP以外限制进程运行的堵塞位，
     * 如果没有了，那么就可以将这个新生儿加入就绪队列了！
     */
    if (old_flags != 0 && proc->flags == 0) {
        lock_ready(proc);
    }
//    else kprintf("old_flag:%d \n",old_flags);

    return OK;
}

/**
 * 获取一个进程的MemoryMap信息
 * @param proc_nr 进程号
 * @param map 要存放的位置
 * @return
 */
PUBLIC int get_mem_map(int proc_nr,MemoryMap *map){
    register Process *proc;

    if(!is_ok_proc_nr(proc_nr)) panic("proc nr err\n",proc_nr);
    proc = proc_addr(proc_nr);      /* 得到进程实例，里面有我们需要的内存映像 */
    map->base=proc->map.base;
    map->size=proc->map.size;
    return OK;
}
