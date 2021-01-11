//
// Created by 杜科 on 2021/1/5.
//
#include <core/fs.h>
#include <core/global.h>
#include <core/dev.h>
#include "stdio.h"

extern u8_t *fsbuf;
extern const int FSBUF_SIZE;
extern Message fs_msg;

/*****************************************************************************
 *                                do_rdwt
 *****************************************************************************/
/**
 * Read/Write file and return byte count read/written.
 *
 * Sector map is not needed to update, since the sectors for the file have been
 * allocated and the bits are set when the file was created.
 * 
 * @return How many bytes have been read/written.
 *****************************************************************************/
PUBLIC int do_rdwt() {
    int fd = fs_msg.FD;    /**< file descriptor. */
    void *buf = fs_msg.BUF;/**< r/w buffer */
    int len = fs_msg.COUNT;    /**< r/w bytes */

    int src = fs_msg.source;        /* caller proc nr. */

//	assert((pcaller->filp[fd] >= &f_desc_table[0]) &&(pcaller->filp[fd] < &f_desc_table[NR_FILE_DESC]));
    if (!(pcaller->filp[fd] >= &f_desc_table[0]) || !(pcaller->filp[fd] < &f_desc_table[NR_FILE_DESC]))
        panic("fd err\n", fd);
    if (!(pcaller->filp[fd]->fd_mode & O_RDWR))
        return 0;

    int pos = pcaller->filp[fd]->fd_pos;

    struct inode *pin = pcaller->filp[fd]->fd_inode;

//	assert(pin >= &inode_table[0] && pin < &inode_table[NR_INODE]);
    if (pin < &inode_table[0] || pin >= &inode_table[NR_INODE]) panic("pin err\n", PANIC_ERR_NUM);

    int imode = pin->i_mode & I_TYPE_MASK;

    if (imode == I_CHAR_SPECIAL) {
        int t = fs_msg.type == READ ? DEVICE_READ : DEVICE_WRITE;
        fs_msg.type = t;

        int dev = pin->i_start_sect;
//		assert(MAJOR(dev) == 4);
//        assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
        if (MAJOR(dev) != 4) panic("dev major err\n", MAJOR(dev));

        fs_msg.DEVICE = MINOR(dev);
        fs_msg.BUF = buf;
        fs_msg.COUNT = len;
        fs_msg.PROC_NR = src;
        send_rec(dd_map[MAJOR(dev)].driver_nr, &fs_msg);
//		assert(fs_msg.COUNT == len);
        if (fs_msg.COUNT != len) panic("fs_msg count err\n", fs_msg.COUNT);

        return fs_msg.COUNT;
    } else {
//		assert(pin->i_mode == I_REGULAR || pin->i_mode == I_DIRECTORY);
        if (pin->i_mode != I_REGULAR && pin->i_mode != I_DIRECTORY) panic("pin err\n", PANIC_ERR_NUM);
//		assert((fs_msg.type == READ) || (fs_msg.type == WRITE));
        if ((fs_msg.type != READ) && (fs_msg.type != WRITE)) panic("fs_msg type err\n", fs_msg.type);

        int pos_end;
        if (fs_msg.type == READ)
            pos_end = MIN(pos + len, pin->i_size);
        else        /* WRITE */
            pos_end = MIN(pos + len, pin->i_nr_sects * SECTOR_SIZE);

        int off = pos % SECTOR_SIZE;
        int rw_sect_min = pin->i_start_sect + (pos >> SECTOR_SIZE_SHIFT);
        int rw_sect_max = pin->i_start_sect + (pos_end >> SECTOR_SIZE_SHIFT);

        int chunk = MIN(rw_sect_max - rw_sect_min + 1,
                        FSBUF_SIZE >> SECTOR_SIZE_SHIFT);

        int bytes_rw = 0;
        int bytes_left = len;
        int i;
        for (i = rw_sect_min; i <= rw_sect_max; i += chunk) {
            /* read/write this amount of bytes every time */
            int bytes = MIN(bytes_left, chunk * SECTOR_SIZE - off);
            rw_sector(DEVICE_READ,
                      pin->i_dev,
                      i * SECTOR_SIZE,
                      chunk * SECTOR_SIZE,
                      FS_TASK,
                      fsbuf);

            if (fs_msg.type == READ) {
                phys_copy((void *) proc_vir2phys(proc_addr(FS_TASK), fsbuf + off),
                          (void *) proc_vir2phys(proc_addr(src), buf + bytes_rw),
                          bytes);

            } else {    /* WRITE */
                phys_copy((void *) proc_vir2phys(proc_addr(src), buf + bytes_rw),
                          (void *) proc_vir2phys(proc_addr(FS_TASK), fsbuf + off),
                          bytes);
                rw_sector(DEVICE_WRITE,
                          pin->i_dev,
                          i * SECTOR_SIZE,
                          chunk * SECTOR_SIZE,
                          FS_TASK,
                          fsbuf);
            }
            off = 0;
            bytes_rw += bytes;
            pcaller->filp[fd]->fd_pos += bytes;
            bytes_left -= bytes;
        }

        if (pcaller->filp[fd]->fd_pos > pin->i_size) {
            /* update inode::size */
            pin->i_size = pcaller->filp[fd]->fd_pos;
            /* write the updated i-node back to disk */
            sync_inode(pin);
        }

        return bytes_rw;
    }
}
