//
// Created by 杜科 on 2020/12/28.
//

#include <core/fs.h>
#include <core/config.h>
#include <core/global.h>
#include <cstring.h>
#include <core/dev.h>
#include <core/hd.h>
#include "stdio.h"

u8_t *fsbuf = (u8_t *) 0x900000;  /* 9M~10M用于文件系统 */
const int FSBUF_SIZE = 0x100000;

Message fs_msg;
PRIVATE int deviceNR; /* 当前文件系统所在的分区，一个分区对应一个次设备。这里去主硬盘中的最大分区 */

PRIVATE void fs_init();

PRIVATE void check_format_FAT32();

PRIVATE void mkfs();

PRIVATE void read_super_block(int dev);

PRIVATE int fs_fork();

PRIVATE int fs_exit();

PUBLIC void fs_task(void) {

    fs_init();
    kprintf("fs_task working...\n");

    while (TRUE) {
        rec(ANY);

        int msgtype = fs_msg.type;
        int src = fs_msg.source;
        pcaller = proc_addr(src);

        switch (msgtype) {
            case OPEN:
                fs_msg.FD = do_open();
                break;
            case CLOSE:
                fs_msg.RETVAL = do_close();
                break;
            case READ:
            case WRITE:
                fs_msg.COUNT = do_rdwt();
                break;
            case UNLINK:
                fs_msg.RETVAL = do_unlink();
                break;
            case RESUME_PROC:
                src = fs_msg.PROC_NR;
                break;
            case FORK:
                fs_msg.RETVAL = fs_fork();
                break;
            case EXIT:
                fs_msg.RETVAL = fs_exit();
                break;
            case LSEEK:
                fs_msg.OFFSET = do_lseek();
                break;
            case STAT:
                fs_msg.RETVAL = do_stat();
                break;
            default:
                kprintf("FS::unknown message:%s\n", &fs_msg);
                panic("wrong msg type\n", msgtype);
                break;
        }

        /* reply */
        if (fs_msg.type != SUSPEND_PROC) {
            fs_msg.type = SYSCALL_RET;
            send(src, &fs_msg);
        }
    }
}

PRIVATE void fs_init() {
    in_outbox(&fs_msg, &fs_msg);

    /* 初始化设备映射驱动 */
    dd_map[0].driver_nr = INVALID_DRIVER;
    dd_map[1].driver_nr = INVALID_DRIVER;
    dd_map[2].driver_nr = INVALID_DRIVER;
    dd_map[3].driver_nr = HD_TASK;        /* 3号主要设备对应硬盘驱动 */
    dd_map[4].driver_nr = TTY_TASK;       /* 4号主要设备对应TTY驱动 */
    dd_map[5].driver_nr = INVALID_DRIVER;

    /* f_desc_table[] */
    for (int i = 0; i < NR_FILE_DESC; i++)
        memset(&f_desc_table[i], 0, sizeof(struct file_desc));

    /* inode_table[] */
    for (int i = 0; i < NR_INODE; i++)
        memset(&inode_table[i], 0, sizeof(struct inode));

    /* super_block[] */
    struct super_block *sb = superBlocks;
    for (; sb < &superBlocks[NR_SUPER_BLOCK]; sb++)
        sb->sb_dev = NO_DEV;

    /* open the device: hard disk */
    fs_msg.type = DEVICE_OPEN;
    fs_msg.DEVICE = MINOR(ROOT_DEV); /* 传入的是次设备号 */
//    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    if (dd_map[MAJOR(ROOT_DEV)].driver_nr == INVALID_DRIVER) panic("the driver_nr is invalid\n", PANIC_ERR_NUM);
    send_rec(dd_map[MAJOR(ROOT_DEV)].driver_nr, &fs_msg);

    /* read the super block of ROOT DEVICE */
    RD_SECT(ROOT_DEV, 1);

    sb = (struct super_block *) fsbuf;
    if (sb->magic != MAGIC_V1)
    { /* 魔数不对，开始格式化 */
        kprintf("{FS} mkfs\n");
        mkfs(); /* make FS */
    }

    /* load super block of ROOT */
    read_super_block(ROOT_DEV);

    sb = get_super_block(ROOT_DEV);
//    assert(sb->magic == MAGIC_V1);
    if (sb->magic != MAGIC_V1) panic("the magic num in super block err", PANIC_ERR_NUM);

    root_inode = get_inode(ROOT_DEV, ROOT_INODE);
}

/**
 *
 * @param io_type DEVICE_READ or DEVICE_WRITE
 * @param dev device nr
 * @param pos Byte offset from/to where to r/w.
 * @param bytes r/w count in bytes.
 * @param proc_nr To whom the buffer belongs.
 * @param buf r/w buffer.
 * @return Zero if success.
 */
PUBLIC int rw_sector(int io_type, int dev, u64_t pos, int bytes, int proc_nr, void *buf) {
    Message driver_msg;

    driver_msg.type = io_type;
    driver_msg.DEVICE = MINOR(dev); /* 传入次设备号 */
    driver_msg.POSITION = pos;
    driver_msg.BUF = buf;
    driver_msg.COUNT = bytes;
    driver_msg.PROC_NR = proc_nr;
//    assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
    if (dd_map[MAJOR(dev)].driver_nr == INVALID_DRIVER) panic("the driver_nr is invalid\n", MAJOR(dev));

    send_rec(dd_map[MAJOR(dev)].driver_nr, &driver_msg);

    return 0;
}

/**
 * Read super block from the given device then write it into a free super_block[] slot.
 * @param dev From which device the super block comes.
 */
PRIVATE void read_super_block(int dev) {
    int i;
    fs_msg.type = DEVICE_READ;
    fs_msg.DEVICE = MINOR(dev);
    fs_msg.POSITION = SECTOR_SIZE * 1;
    fs_msg.BUF = fsbuf;
    fs_msg.COUNT = SECTOR_SIZE;
    fs_msg.PROC_NR = FS_TASK;
//    assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
    if (dd_map[MAJOR(dev)].driver_nr == INVALID_DRIVER) panic("the driver_nr is invalid\n", PANIC_ERR_NUM);
    send_rec(dd_map[MAJOR(dev)].driver_nr, &fs_msg);

    /* find a free slot in super_block[] */
    for (i = 0; i < NR_SUPER_BLOCK; i++)
        if (superBlocks[i].sb_dev == NO_DEV)
            break;
    if (i == NR_SUPER_BLOCK) panic("super_block slots used up", PANIC_ERR_NUM);

//    assert(i == 0); /* currently we use only the 1st slot */
    if (i != 0) panic("super_block slots remaining", PANIC_ERR_NUM);

    struct super_block *psb = (struct super_block *) fsbuf;

    superBlocks[i] = *psb;
    superBlocks[i].sb_dev = dev;
}

/**
 * Get the super block from super_block[].
 * @param dev Device nr.
 * @return Super block ptr.
 */
PUBLIC struct super_block *get_super_block(int dev) {
    struct super_block *sb = superBlocks;
    for (; sb < &superBlocks[NR_SUPER_BLOCK]; sb++)
        if (sb->sb_dev == dev)
            return sb;

    panic("super block of devie %d not found.\n", dev);

    return 0;
}

/*****************************************************************************
 *                                get_inode
 *****************************************************************************/
/**
 * <Ring 1> Get the inode ptr of given inode nr. A cache -- inode_table[] -- is
 * maintained to make things faster. If the inode requested is already there,
 * just return it. Otherwise the inode will be read from the disk.
 *
 * @param dev Device nr.
 * @param num I-node nr.
 *
 * @return The inode ptr requested.
 *****************************************************************************/
PUBLIC struct inode *get_inode(int dev, int num) {
    if (num == 0)
        return 0;

    struct inode *p;
    struct inode *q = 0;
    for (p = &inode_table[0]; p < &inode_table[NR_INODE]; p++) {
        if (p->i_cnt) {    /* not a free slot */
            if ((p->i_dev == dev) && (p->i_num == num)) {
                /* this is the inode we want */
                p->i_cnt++;
                return p;
            }
        } else {        /* a free slot */
            if (!q) /* q hasn't been assigned yet */
                q = p; /* q <- the 1st free slot */
        }
    }

    if (!q) panic("the inode table is full", PANIC_ERR_NUM);

    q->i_dev = dev;
    q->i_num = num;
    q->i_cnt = 1;

    struct super_block *sb = get_super_block(dev);
    int blk_nr = 1 + 1 + sb->nr_imap_sects + sb->nr_smap_sects + ((num - 1) / (SECTOR_SIZE / INODE_SIZE));
    RD_SECT(dev, blk_nr);
    struct inode *pinode = (struct inode *) ((u8_t *) fsbuf + ((num - 1) % (SECTOR_SIZE / INODE_SIZE)) * INODE_SIZE);
    q->i_mode = pinode->i_mode;
    q->i_size = pinode->i_size;
    q->i_start_sect = pinode->i_start_sect;
    q->i_nr_sects = pinode->i_nr_sects;
    return q;
}

/*****************************************************************************
 *                                put_inode
 *****************************************************************************/
/**
 * Decrease the reference nr of a slot in inode_table[]. When the nr reaches
 * zero, it means the inode is not used any more and can be overwritten by
 * a new inode.
 *
 * @param pinode I-node ptr.
 *****************************************************************************/
PUBLIC void put_inode(struct inode *pinode) {
//    assert(pinode->i_cnt > 0);
    if (pinode->i_cnt <= 0) panic("pinode i_cnt err\n", PANIC_ERR_NUM);
    pinode->i_cnt--;
}


/**
 * Make a available FS in the disk. It will
 * - Write a super block to sector 1.
 * - Create three special files: dev_tty0, dev_tty1, dev_tty2
 * - Create a file cmd.tar
 * - Create the inode map
 * - Create the sector map
 * - Create the inodes of the files
 * - Create `/', the root directory
 */
PRIVATE void mkfs() {
    int i, j;

    /* super block */
    /* get the geometry of ROOT_DEV */
    Partition geo;
    fs_msg.type = DEVICE_IOCTL;
    fs_msg.DEVICE = MINOR(ROOT_DEV);
    fs_msg.REQUEST = DIOCTL_GET_GEO;
    fs_msg.BUF = &geo;
    fs_msg.PROC_NR = FS_TASK;
//    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    if (dd_map[MAJOR(ROOT_DEV)].driver_nr == INVALID_DRIVER) panic("the driver_nr is invalid\n", PANIC_ERR_NUM);
    send_rec(dd_map[MAJOR(ROOT_DEV)].driver_nr, &fs_msg);

    kprintf("{FS} dev size: 0x%x sectors\n", geo.size);

    int bits_per_sect = SECTOR_SIZE * 8; /* 8 bits per byte */
    /* generate a super block */
    struct super_block sb;
    sb.magic = MAGIC_V1; /* 0x111 */
    sb.nr_inodes = bits_per_sect; /* 512*8=4096，即最多支持4096个文件 */
    sb.nr_inode_sects = sb.nr_inodes * INODE_SIZE / SECTOR_SIZE; /* inode占的扇区数 */
    sb.nr_sects = geo.size; /* partition size in sector */
    sb.nr_imap_sects = 1;
    sb.nr_smap_sects = sb.nr_sects / bits_per_sect + 1; /* sector-map占的扇区数 */
    sb.nr_db_sect = 1 + 1 +   /* boot sector & super block */
                    sb.nr_imap_sects + sb.nr_smap_sects + sb.nr_inode_sects; /* 数据起始扇区号 */
    sb.root_inode = ROOT_INODE;
    sb.inode_size = INODE_SIZE;
    struct inode x;
    sb.inode_isize_off = (int) &x.i_size - (int) &x;
    sb.inode_start_off = (int) &x.i_start_sect - (int) &x;
    sb.dir_ent_size = DIR_ENTRY_SIZE;
    struct dir_entry de;
    sb.dir_ent_inode_off = (int) &de.inode_nr - (int) &de;
    sb.dir_ent_name_off = (int) &de.name - (int) &de;

    memset(fsbuf, 0x90, SECTOR_SIZE); /* 这一扇区内容后面填充0x90 */
    memcpy(fsbuf, &sb, SUPER_BLOCK_SIZE);

    /* write the super block */
    WR_SECT(ROOT_DEV, 1);

    kprintf("{FS} devbase:0x%x00, sb:0x%x00, imap:0x%x00, smap:0x%x00\n"
            "        inodes:0x%x00, 1st_sector:0x%x00\n",
            geo.base * 2,
            (geo.base + 1) * 2,
            (geo.base + 1 + 1) * 2,
            (geo.base + 1 + 1 + sb.nr_imap_sects) * 2,
            (geo.base + 1 + 1 + sb.nr_imap_sects + sb.nr_smap_sects) * 2,
            (geo.base + sb.nr_db_sect) * 2);

    /* 设置索引节点映射 */
    memset(fsbuf, 0, SECTOR_SIZE);
    /* free, /, dev_tty0, dev_tty1,  dev_tty2, cmd.tar */
    for (i = 0; i < (NR_CONSOLES + 3); i++)
        fsbuf[0] |= 1 << i;

    if (fsbuf[0] != 0x3F) panic("fsbuf[0] err\n", PANIC_ERR_NUM);
//    assert(fsbuf[0] == 0x3F);
    /** 0011 1111 :
     *    || ||||
     *    || |||`--- bit 0 : reserved
     *    || ||`---- bit 1 : the first inode, which indicates `/'
     *    || |`----- bit 2 : /dev_tty0
     *    || `------ bit 3 : /dev_tty1
     *    |`-------- bit 4 : /dev_tty2
     *    `--------- bit 5 : /cmd.tar
     */
    WR_SECT(ROOT_DEV, 2); /* 第二个扇区为inode-map */

    /* 设置扇区映射 */
    memset(fsbuf, 0, SECTOR_SIZE);
    int nr_sects = NR_DEFAULT_FILE_SECTS + 1; /* 初始时，已使用扇区 */
    /**
     *             ~~~~~~~~~~~~~~~~~~~|~   |
     *                                |    `--- bit 0 is reserved
     *                                `-------- for `/'
     */
    for (i = 0; i < nr_sects / 8; i++)
        fsbuf[i] = 0xFF;

    for (j = 0; j < nr_sects % 8; j++)
        fsbuf[i] |= (1 << j);

    WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects); /* inode-map接着是sector-map */

    /* 用零填充剩余的扇区映射 */
    memset(fsbuf, 0, SECTOR_SIZE);
    for (i = 1; i < sb.nr_smap_sects; i++)
        WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + i);

    /* 创建文件cmd.tar */
    /* make sure it'll not be overwritten by the disk log */
//    assert(INSTALL_START_SECT + INSTALL_NR_SECTS < sb.nr_sects - NR_SECTS_FOR_LOG);
    if (INSTALL_START_SECT + INSTALL_NR_SECTS >= sb.nr_sects - NR_SECTS_FOR_LOG)
        panic("install sec err\n", INSTALL_START_SECT + INSTALL_NR_SECTS);
    int bit_offset = INSTALL_START_SECT - sb.nr_db_sect + 1; /* sect M <-> bit (M - sb.n_1stsect + 1) */
    int bit_off_in_sect = bit_offset % (SECTOR_SIZE * 8);
    int bit_left = INSTALL_NR_SECTS;
    int cur_sect = bit_offset / (SECTOR_SIZE * 8);
    RD_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + cur_sect);
    while (bit_left) {
        int byte_off = bit_off_in_sect / 8;
        /* this line is ineffecient in a loop, but I don't care */
        fsbuf[byte_off] |= 1 << (bit_off_in_sect % 8);
        bit_left--;
        bit_off_in_sect++;
        if (bit_off_in_sect == (SECTOR_SIZE * 8)) {
            WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + cur_sect);
            cur_sect++;
            RD_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + cur_sect);
            bit_off_in_sect = 0;
        }
    }
    WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + cur_sect);

    /* 设置索引节点 */
    /* inode of `/' */
    memset(fsbuf, 0, SECTOR_SIZE);
    struct inode *pi = (struct inode *) fsbuf;
    pi->i_mode = I_DIRECTORY;
    pi->i_size = DIR_ENTRY_SIZE * 5; /* 5 files:`.', `dev_tty0', `dev_tty1', `dev_tty2', `cmd.tar' */
    pi->i_start_sect = sb.nr_db_sect;
    pi->i_nr_sects = NR_DEFAULT_FILE_SECTS;
    /* inode of `/dev_tty0~2' */
    for (i = 0; i < NR_CONSOLES; i++) {
        pi = (struct inode *) (fsbuf + (INODE_SIZE * (i + 1)));
        pi->i_mode = I_CHAR_SPECIAL;
        pi->i_size = 0;
        pi->i_start_sect = MAKE_DEV(DEV_CHAR_TTY, i);
        pi->i_nr_sects = 0;
    }
    /* inode of `/cmd.tar' */
    pi = (struct inode *) (fsbuf + (INODE_SIZE * (NR_CONSOLES + 1)));
    pi->i_mode = I_REGULAR;
    pi->i_size = INSTALL_NR_SECTS * SECTOR_SIZE;
    pi->i_start_sect = INSTALL_START_SECT;
    pi->i_nr_sects = INSTALL_NR_SECTS;
    WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + sb.nr_smap_sects);

    /* 创建出根目录文件 */
    memset(fsbuf, 0, SECTOR_SIZE);
    struct dir_entry *pde = (struct dir_entry *) fsbuf;

    pde->inode_nr = ROOT_INODE; /* 根目录inode索引为1 */
    strcpy(pde->name, ".");

    /* dir entries of `/dev_tty0~2' */
    for (i = 0; i < NR_CONSOLES; i++) {
        pde++;
        pde->inode_nr = i + 2; /* dev_tty0's inode_nr is 2 */
//        sprintf(pde->name, "dev_tty%d", i);
        switch (i) {
            case 0:
                strcpy(pde->name, "dev_tty0");
                break;
            case 1:
                strcpy(pde->name, "dev_tty1");
                break;
            case 2:
                strcpy(pde->name, "dev_tty2");
                break;
        }
    }

    /* 最后是'cmd.tar'的 */
    (++pde)->inode_nr = NR_CONSOLES + 2;
//    sprintf(pde->name, "cmd.tar", i);
    strcpy(pde->name, "cmd.tar");
    WR_SECT(ROOT_DEV, sb.nr_db_sect);
}

/*****************************************************************************
 *                                sync_inode
 *****************************************************************************/
/**
 * <Ring 1> Write the inode back to the disk. Commonly invoked as soon as the
 *          inode is changed.
 *
 * @param p I-node ptr.
 *****************************************************************************/
PUBLIC void sync_inode(struct inode *p) {
    struct inode *pinode;
    struct super_block *sb = get_super_block(p->i_dev);
    int blk_nr = 1 + 1 + sb->nr_imap_sects + sb->nr_smap_sects +
                 ((p->i_num - 1) / (SECTOR_SIZE / INODE_SIZE));
    RD_SECT(p->i_dev, blk_nr);
    pinode = (struct inode *) ((u8_t *) fsbuf +
                               (((p->i_num - 1) % (SECTOR_SIZE / INODE_SIZE))
                                * INODE_SIZE));
    pinode->i_mode = p->i_mode;
    pinode->i_size = p->i_size;
    pinode->i_start_sect = p->i_start_sect;
    pinode->i_nr_sects = p->i_nr_sects;
    WR_SECT(p->i_dev, blk_nr);
}

PRIVATE int fs_fork(){
    int i;
    Process * child = gp_procs[fs_msg.PID];
    for (i = 0; i < NR_FILES; i++) {
        if (child->filp[i]) {
            child->filp[i]->fd_cnt++;
            child->filp[i]->fd_inode->i_cnt++;
        }
    }

    return 0;
}

PRIVATE int fs_exit() {
    int i;
    Process *p = gp_procs[fs_msg.PID];
    for (i = 0; i < NR_FILES; i++) {
        if (p->filp[i]) {
            /* release the inode */
            p->filp[i]->fd_inode->i_cnt--;
            /* release the file desc slot */
            if (--p->filp[i]->fd_cnt == 0)
                p->filp[i]->fd_inode = 0;
            p->filp[i] = 0;
        }
    }
    return 0;
}

PRIVATE void test_rw() {
    //    /* 打开0号主设备 */
//    msg.source = FS_TASK;
//    msg.type = DEVICE_OPEN;
//    msg.DEVICE = 0;
//    send_rec(HD_TASK, &msg);
//    deviceNR = msg.REPLY_LARGEST_PART_NR;
//    kprintf("<fs>: cur device num is:%d\n", deviceNR);
//
//    /* 9MB~10MB: buffer for FS */
//    fsbuf = (u8_t *) 0x900000;
//
//    deviceNR=ROOT_DEV;
//
//    RD_SECT(deviceNR,0);
//    for (int i = 0; i < 512; ++i) {
//        kprintf("%d ",*fsbuf);
//        fsbuf++;
//    }
//    kprintf("\n");
//
//    fsbuf = (u8_t *) 0x900000;
//    for (int i = 0; i < 512; ++i) {
//        *fsbuf=i;
//        fsbuf++;
//    }
//    fsbuf = (u8_t *) 0x900000;
//    WR_SECT(deviceNR,0);
//
//    for (int i = 0; i < 512; ++i) {
//        *fsbuf=0;
////        kprintf("%d ",*fsbuf);
//        fsbuf++;
//    }
//    kprintf("\n");
//
//    fsbuf = (u8_t *) 0x900000;
//    RD_SECT(deviceNR,0);
//    for (int i = 0; i < 512; ++i) {
////        kprintf("%d ",*fsbuf);
//        fsbuf++;
//    }
//    kprintf("\n");
}

/* 读取分析当前分区的引导扇区，检查是否为FAT32文件系统，如果不是，那么将当前分区格式化为FAT32文件系统 */
PRIVATE void check_format_FAT32() {

}