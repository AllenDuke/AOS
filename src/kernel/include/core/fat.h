//
// Created by 杜科 on 2020/12/29.
//

#ifndef AOS_FAT_H
#define AOS_FAT_H

 /* FAT文件系统的BPB结构，512字节 */
typedef struct bpb_t {
    u8_t BS_jmpBoot[3];             /* 一个短跳转指令 */
    u8_t BS_OEMName[8];             /* 厂商名 */
    u16_t BPB_BytsPerSec;           /* 每扇区字节数 */
    u8_t BPB_SecPerClus;            /* 每簇扇区数 */
    u16_t BPB_RsvdSecCnt;           /* 保留区扇区数，即FAT1前的扇区数 */
    u8_t BPB_NumFATs;               /* FAT表数，通常为2 */
    u16_t BPB_RootEntCnt;           /* FAT32必须等于0，FAT12/FAT16为根目录中目录项的个数 */
    u16_t BPB_TotSec16;             /* FAT32必须等于0，FAT12/FAT16为扇区总数 */
    u8_t BPB_Media;                 /* 存储介质类型，0xF8标准值，可移动存储介质，常用的 0xF0 */
    u16_t BPB_FATSz16;              /* FAT32必须为0，FAT12/FAT16为每个FAT表所占的扇区数。 */
    u16_t BPB_SecPerTrk;            /* 每磁道扇区数 */
    u16_t BPB_NumHeads;             /* 磁头数 */
    u32_t BPB_HiddSec;              /* 隐藏扇区数 */
    u32_t BPB_TotSec32;             /* 扇区总数，上面的BPB_TotSec16为0，由这个来记录扇区总数 */
    u32_t BPB_FATSz32;              /* 每个FAT表所占扇区数 */
    u16_t BPB_ExtFlags;             /* 扩展标记 */
    u16_t BPB_FSVer;                /* 版本号 */
    u32_t BPB_RootClus;             /* 根目录所在第一个簇的簇号，通常情况下还是起始于2号簇 */
    u16_t BPB_FsInfo;               /* FSINFO（文件系统信息扇区）扇区号1，该扇区为操作系统提供关于空簇总数及下一可用簇的信息 */
    u16_t BPB_BkBootSec;            /* 备份引导扇区的扇区号。备份引导扇区总是位于文件系统 的6号扇区*/
    u8_t BPB_Reserved[12];          /* 用于以后FAT 扩展使用 */
    u8_t BS_DrvNum;                 /* 设备号 */
    u8_t BS_Reserved1;
    u8_t BS_BootSig;                /* 扩展引导标志，0x29 */
    u32_t BS_VolID;                 /* 卷序列号，通常为一个随机值*/
    u8_t BS_VolLab[11];             /* 卷标名称 */
    u8_t BS_FileSysType[8];         /* 文件系统类型名称 */
    u8_t notUse[410];               /* 引导代码，不作为启动盘时未使用 */
    u16_t endFlags;                 /* 0XAA55 */
} BPB;

/* FAT目录项日期 2字节 */
typedef struct dir_entry_date_t {
    u16_t day : 5;                  /* 低5位 日 */
    u16_t month : 4;                /* 中4位 月 */
    u16_t yearFrom1980 : 7;         /* 高7位 年 */
} DirEntryDate;

/* FAT目录项时间 2字节 */
typedef struct dir_entry_time_t {
    u16_t second2 : 5;              /* 低5位 2秒 */
    u16_t minute : 6;               /* 中6位 分 */
    u16_t hour : 5;                 /* 高5位 时 */
} DirEntryTime;

/* 文件属性 */
#define FILE_ATTR_RW        0       /* 读写 */
#define FILE_ATTR_R         1       /* 只读 */
#define FILE_ATTR_HIDE      2       /* 隐藏文件 */
#define FILE_ATTR_SYS       4       /* 系统文件 */
#define FILE_ATTR_VOL_LAB   8       /* 卷标 分区 */
#define FILE_ATTR_SUB_DIR   16      /* 子目录 */
#define FILE_ATTR_TAR       32      /* 归档文件 */

/* FAT目录项 32字节 */
typedef struct dir_entry_t {
    u8_t name[8];                   /* 文件名 */
    u8_t extName[3];                /* 扩展名 */
    u8_t attr;                      /* 属性 */
    u8_t notUse;                    /* 保留未使用 */
    u8_t crtTimeMS10;               /* 创建时间 10毫秒 */
    DirEntryTime crtTime;           /* 创建时间 */
    DirEntryDate crtDate;           /* 创建日期 */
    DirEntryDate lastAccDate;       /* 最后访问日期 */
    u16_t beginClusterH;            /* 文件开始簇号高16位 */
    DirEntryTime updateTime;        /* 修改时间 */
    DirEntryDate updateDate;        /* 修改日期 */
    u16_t beginClusterL;            /* 文件开始簇号低16位 */
    u32_t fileSize;                 /* 文件字节大小 */
} DirEntry;

#endif //AOS_FAT_H
