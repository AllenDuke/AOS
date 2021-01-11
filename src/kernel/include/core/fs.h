//
// Created by 杜科 on 2020/12/29.
//

#ifndef AOS_FS_H
#define AOS_FS_H

#include "fat.h"

/**
 * @struct dev_drv_map fs.h "include/sys/fs.h"
 * @brief  The Device_nr.\ - Driver_nr.\ MAP.
 */
struct dev_drv_map {
    int driver_nr; /**< The proc nr.\ of the device driver. */
};

/**
 * @def   MAGIC_V1
 * @brief Magic number of FS v1.0
 */
#define    MAGIC_V1    0x111

/**
 * @struct super_block fs.h "include/fs.h"
 * @brief  The 2nd sector of the FS
 *
 * Remember to change SUPER_BLOCK_SIZE if the members are changed.
 *
 * 当前分区中，每个次设备都经对应一个超级块，例如有可能通过mount挂载其他次设备
 */
struct super_block {
    u32_t magic;              /* Magic number */
    u32_t nr_inodes;          /* How many inodes，这里512*8=4096，这里用一个扇区来存放inode-map（表示4096个inode的使用情况） */
    u32_t nr_sects;           /* How many sectors  文件系统占的扇区数 */
    u32_t nr_imap_sects;      /* inode-map占的扇区数 这里1扇区 */
    u32_t nr_smap_sects;      /* sector-map占的扇区数 */
    u32_t nr_db_sect;         /* 数据起始扇区号 */
    u32_t nr_inode_sects;     /* 4096个inode占的扇区数 */
    u32_t root_inode;         /* 根node索引号 */
    u32_t inode_size;         /* INODE_SIZE */
    u32_t inode_isize_off;    /* i_size在inode中的偏移量 */
    u32_t inode_start_off;    /* i_start_sect在inode中的偏移量  */
    u32_t dir_ent_size;       /* DIR_ENTRY_SIZE */
    u32_t dir_ent_inode_off;  /* inode_nr在dir_entry的偏移量 */
    u32_t dir_ent_name_off;   /* name在dir_entry的偏移量 */

    /*
     * the following item(s) are only present in memory
     */
    int sb_dev;                /* the super block's home device */
};

/**
 * @def   SUPER_BLOCK_SIZE
 * @brief The size of super block \b in \b the \b device.
 *
 * Note that this is the size of the struct in the device, \b NOT in memory.
 * The size in memory is larger because of some more members.
 */
#define SUPER_BLOCK_SIZE    (4 * 14)    /* 这是设备中结构的大小，不是内存中的 */

/**
 * @struct inode
 * @brief  i-node
 *
 * The \c start_sect and\c nr_sects locate the file in the device,
 * and the size show how many bytes is used.
 * If <tt> size < (nr_sects * SECTOR_SIZE) </tt>, the rest bytes
 * are wasted and reserved for later writing.
 *
 * \b NOTE: Remember to change INODE_SIZE if the members are changed
 *
 * 一个inode对应一个文件，44字节
 */
struct inode {
    u32_t i_mode;           /* Accsess mode 目录or普通文件or特殊字符设备 */
    u32_t i_size;           /* File size */
    u32_t i_start_sect;     /* The first sector of the data */
    u32_t i_nr_sects;       /* How many sectors the file occupies */
    u8_t _unused[16];       /* Stuff for alignment */

    /* the following items are only present in memory */
    int i_dev;
    int i_cnt;        /* How many procs share this inode  */
    int i_num;        /* inode nr.  */
};

/**
 * @def   INODE_SIZE
 * @brief The size of i-node stored \b in \b the \b device.
 *
 * Note that this is the size of the struct in the device, \b NOT in memory.
 * The size in memory is larger because of some more members.
 */
#define INODE_SIZE    32

/**
 * @def   MAX_FILENAME_LEN
 * @brief Max len of a filename
 * @see   dir_entry
 */
#define MAX_FILENAME_LEN    12

/**
 * @struct dir_entry
 * @brief  Directory Entry
 */
struct dir_entry {
    int inode_nr;                   /* inode nr. */
    char name[MAX_FILENAME_LEN];    /* Filename */
};

/**
 * @def   DIR_ENTRY_SIZE
 * @brief The size of directory entry in the device.
 *
 * It is as same as the size in memory.
 */
#define DIR_ENTRY_SIZE    sizeof(struct dir_entry)

/**
 * @struct file_desc
 * @brief  File Descriptor 文件描述符
 */
struct file_desc {
    int fd_mode;               /* R or W */
    int fd_pos;                /* Current position for R/W. */
    int fd_cnt;                /* How many procs share this desc */
    struct inode *fd_inode;    /* Ptr to the i-node */
};

/**
 * Since all invocations of `rw_sector()' in FS look similar (most of the
 * params are the same), we use this macro to make code more readable.
 */
#define RD_SECT(dev, sect_nr) rw_sector(DEVICE_READ, \
                       dev,                \
                       (sect_nr) * SECTOR_SIZE,        \
                       SECTOR_SIZE, /* read one sector */ \
                       FS_TASK,                \
                       fsbuf);
#define WR_SECT(dev, sect_nr) rw_sector(DEVICE_WRITE, \
                       dev,                \
                       (sect_nr) * SECTOR_SIZE,        \
                       SECTOR_SIZE, /* write one sector */ \
                       FS_TASK,                \
                       fsbuf);

#define	is_special(m)	((((m) & I_TYPE_MASK) == I_BLOCK_SPECIAL) ||	\
			 (((m) & I_TYPE_MASK) == I_CHAR_SPECIAL))


#endif //AOS_FS_H
