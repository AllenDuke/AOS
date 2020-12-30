//
// Created by 杜科 on 2020/12/27.
//

#ifndef AOS_DEV_H
#define AOS_DEV_H

#define DIOCTL_GET_GEO          1
#define DIOCTL_SET_GEO          2

/* 硬盘驱动器 */
#define SECTOR_SIZE             512                 /* 扇区大小（字节） */
#define SECTOR_BITS             (SECTOR_SIZE * 8)   /* 扇区大小（位） */
#define SECTOR_SIZE_SHIFT       9                   /* 扇区大小移位 */
#define SECTOR_MASK             SECTOR_SIZE - 1     /* 扇区掩码（边界） */

#define MAX_DRIVES              2                   /* 最多支持2个物理硬盘 */
#define NR_PART_PER_DRIVE       4                   /* 每个物理磁盘最多4个分区，一般为P+P+P+P或P+P+P+E */
#define NR_SUB_PER_PART         16                  /* 扩展分区最多扩展为16个逻辑驱动器 */
#define NR_SUB_PER_DRIVE        (NR_SUB_PER_PART * NR_PART_PER_DRIVE)   /* 逻辑分区表项总数 */
#define NR_PRIM_PER_DRIVE       (NR_PART_PER_DRIVE + 1)


#define    MAX_PRIM        (MAX_DRIVES * NR_PRIM_PER_DRIVE - 1)

#define    MAX_SUBPARTITIONS    (NR_SUB_PER_DRIVE * MAX_DRIVES)

/* 主要设备编号*/                 /* 0不存在 */
#define    DEV_FLOPPY            1   /* 软盘驱动 */
#define    DEV_CDROM            2   /* 光盘驱动 */
#define    DEV_HD                3   /* AT硬盘驱动 */
#define    DEV_CHAR_TTY        4   /* 字符终端设备 */
#define    DEV_SCSI            5   /* SCSI接口硬盘 */

/* 硬盘次设备号 */
#define    MINOR_hd1a        0x10
#define    MINOR_hd2a        (MINOR_hd1a + NR_SUB_PER_PART)
#define    MINOR_hd2c        (MINOR_hd1a + NR_SUB_PER_PART + 2)
/* boot的次设备号
 * 对应于src/boot/load.inc::ROOT_BASE，如果更改，请连同一起更改。
 */
#define    MINOR_BOOT            MINOR_hd2c

/* 根据主号和次号进行设备编号 */
#define    MAJOR_SHIFT            8
#define    MAKE_DEV(a, b)        ((a << MAJOR_SHIFT) | b)
#define    ROOT_DEV        MAKE_DEV(DEV_HD, MINOR_BOOT)

/* 这两个宏可以将主次设备号分开 */
#define    MAJOR(x)        ((x >> MAJOR_SHIFT) & 0xFF)
#define    MINOR(x)        (x & 0xFF)

#define    P_PRIMARY    0
#define    P_EXTENDED    1

#define AOS_PART    102        /* AOS的分区结构 */
#define NO_PART            0x00    /* 未使用的条目 */
#define EXT_PART        0x05    /* 扩展分区 */


#define DEVICE_READ          3    /* 终端功能索引代码，从终端读取数据 */
#define DEVICE_WRITE         4    /* 终端功能索引代码，写入数据到终端 */
#define DEVICE_IOCTL         5    /* 终端功能索引代码，终端io控制 */
#define DEVICE_OPEN          6    /* 终端功能索引代码， */
#define DEVICE_CLOSE         7    /* 终端功能索引代码，关闭一个终端设备  */
#define DEVICE_SCATTER       8    /* fcn code for writing from a vector */
#define DEVICE_GATHER        9    /* fcn code for reading into a vector */

#endif //AOS_DEV_H
