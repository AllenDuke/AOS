//
// Created by 杜科 on 2020/12/28.
//

#include "core/kernel.h"

PRIVATE Message msg;
PRIVATE int deviceNR; /* 当前文件系统所在的分区，一个分区对应一个设备。这里去主硬盘中的最大分区 */

PRIVATE void fs_init();
PRIVATE void check_format_FAT32();


PUBLIC void fs_task(void){

    kprintf("fs_task working...\n");
    fs_init();
    kprintf("<fs>: cur device num is:%d\n",deviceNR);

    while (TRUE){
        rec(ANY);
    }
}

PRIVATE void fs_init(){
    in_outbox(&msg,&msg);
    /* 打开0号设备 */
    msg.source=FS_TASK;
    msg.type=DEVICE_OPEN;
    msg.DEVICE=0;
    send_rec(HD_TASK,&msg);
    deviceNR=msg.REPLY_LARGEST_PRIM_PART_NR;
}

/* 读取分析当前分区的引导扇区，检查是否为FAT32文件系统，如果不是，那么将当前分区格式化为FAT32文件系统 */
PRIVATE void check_format_FAT32(){

}