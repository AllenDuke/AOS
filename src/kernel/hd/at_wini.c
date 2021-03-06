//
// Created by 杜科 on 2020/12/27.
//

#include <core/config.h>
#include <core/global.h>
#include <errno.h>
#include <cstring.h>
#include <ibm/partition.h>
#include <core/hd.h>
#include "stdio.h"

PRIVATE int nr_drives;                  /* 磁盘驱动器的数量 */
PRIVATE bool_t intr_open;               /* 中断打开状态，1开0关 */
PRIVATE Message msg;                    /* 通信消息 */

PRIVATE u8_t wini_status;               /* 中断完成后的状态 */
PRIVATE u8_t hdbuf[SECTOR_SIZE * 2];    /* 硬盘缓冲区，DMA也用它 */
PRIVATE phys_addr hdbuf_phys;           /* 缓冲区的物理地址 */
PRIVATE HDInfo hd_info[1];              /* 暂时只支持一个硬盘... */

PRIVATE int largestPrimDeviceNR = 0;    /*  最大块主分区索引，不包括扩展分区 */
PRIVATE int largestLogicDeviceNR = 0;   /*  最大块逻辑分区索引 */

/* 得到次设备的驱动程序，一个分区对应一个次设备 */
#define DRIVER_OF_DEVICE(dev) (dev <= MAX_PRIM ? dev / NR_PRIM_PER_DRIVE  /* 是物理分区 */\
: (dev - MINOR_hd1a) / NR_SUB_PER_DRIVE) /* 是逻辑分区 */

PRIVATE void init_params(void);

PRIVATE char *wini_name(void);

PRIVATE int wini_do_open(int device);

PRIVATE int wini_identify(int drive);

PRIVATE int wini_do_close(int device);

PRIVATE int wini_do_readwrite(Message *msg);

PRIVATE int wini_do_vreadwrite(Message *msg);

PRIVATE int wini_do_ioctl(Message *msg);

PRIVATE void wini_geometry(Partition *partition);

PRIVATE int wini_prepar(int device);

PRIVATE int wini_handler(int irq);

PRIVATE int cmd_out(Command *cmd);

PRIVATE int wini_wait_for(int mask, int value);

PRIVATE clock_t get_uptime();

PRIVATE void wini_interrupt_wait(void);

PRIVATE void wini_print_identify_info(u16_t *hdinfo);

PRIVATE void partition(int device, int style);

PRIVATE void get_part_table(int drive, int sect_nr, PartEntry *entry);

PUBLIC void at_winchester_task(void) {
    int rs, caller, proc_nr;

    init_params();
    kprintf("{HD}-> Drives count: %d\n", nr_drives);
    kprintf("{HD}-> Hard Disk Driver Working...\n");
    /* 驱动程序开始工作了 */
    while (TRUE) {
        /* 等待外界的消息 */
        receive(ANY, &msg);
//        kprintf("at_winchester_task got msg\n");
        /* 得到请求者以及需要服务的进程编号 */
        caller = msg.source;
        proc_nr = msg.PROC_NR;

        /* 检查请求者是否合法：只能是文件系统或者其他的系统任务 */
        if (caller != FS_TASK && caller >= 0) {
            kprintf("%s: got message form %d\n", wini_name(), caller);
            continue;   /* 重新等待工作 */
        }

        /* 现在根据请求做事情 */
        switch (msg.type) {
            /* DEVICE_OPEN(打开)、DEVICE_CLOSE(关闭)、DEVICE_IOCTL（设备io控制） */
            case DEVICE_OPEN:
                rs = wini_do_open(msg.DEVICE);
                /* 恢复当前驱动器内，容量最大的分区号 */
                if (hd_info[0].primary[largestPrimDeviceNR].size > hd_info[0].logical[largestLogicDeviceNR].size)
                    msg.REPLY_LARGEST_PART_NR = largestPrimDeviceNR;
                else msg.REPLY_LARGEST_PART_NR = largestLogicDeviceNR;
                break;
            case DEVICE_CLOSE:
                rs = wini_do_close(msg.DEVICE);
                break;
            case DEVICE_IOCTL:
                rs = wini_do_ioctl(&msg);
                break;

                /* 而DEVICE_READ（读数据）、 DEV_WRITE（写数据）和剩下的两个操作
                 */
            case DEVICE_READ:
            case DEVICE_WRITE:
                rs = wini_do_readwrite(&msg);
                break;

            case DEVICE_GATHER:
            case DEVICE_SCATTER:
                rs = wini_do_vreadwrite(&msg);
                break;

                /* 被中断唤醒或闹钟唤醒 */
            case HARD_INT:
                continue;

                /* 能力之外，爱莫能助 */
            default:
                rs = EINVAL;
                break;
        }

        /*  最后，给请求者一个答复 */
        msg.type = 60;
        msg.REPLY_PROC_NR = proc_nr;
        msg.REPLY_STATUS = rs;          /* 传输的字节数或错误代码 */
        send(caller, &msg);             /* 走你 */
    }
}

/**
 * 初始化硬盘参数
 */
PRIVATE void init_params(void) {
    /**
     * 由于在必须使用设备以前对设备初始化可能会失败，所以在内核初始化时
     * 调用本函数并不做任何访问磁盘的工作。它做的主要工作是把有关磁盘的逻辑
     * 配置信息拷贝到params数组中。而这些信息是ROM BIOS从CMOS存储器中提取的，
     * "奔腾"类计算机在这些存储器中存放配置信息。
     * 当机器第一次接通电源时，在AOS第一部分的装载过程开始以前，执行BIOS
     * 中的功能。取不出这项信息并不是致命的，如果磁盘是现代化的磁盘，
     * 该信息可以从磁盘直接得到。
     */

    int i;
    u8_t params[16];
    phys_addr param_phys = vir2phys(params);
    /* 从BIOS数据区域获取磁盘驱动器的数量 */
    phys_copy(0x475L, param_phys, 1L);
    if ((nr_drives = params[0]) > 2) nr_drives = 2;  /* 就算磁盘驱动器>2，我们也只用两个 */
    if (nr_drives == 0) {     /* 没有硬盘 */
        panic("AOS Cannot continue, Because no any HardDisks on pc.", PANIC_ERR_NUM);
    }

    /* 初始化并得到DMA缓冲区的物理地址 */
    hdbuf_phys = vir2phys(&hdbuf);

    /* 初始化硬盘参数 */
    for (i = 0; i < (sizeof(hd_info) / sizeof(hd_info[0])); i++) {
        memset(&hd_info[i], 0, sizeof(hd_info[0]));
    }
    intr_open = FALSE;  /* 现在不设置中断，可能会失败 */
}

/**
 * 打开硬盘设备
 * @param device 次设备号
 * @return
 */
PRIVATE int wini_do_open(int device) {
    int drive = DRIVER_OF_DEVICE(device);
    if (drive != 0) panic("device only be 0\n", drive); /* 现在只能处理一个硬盘，所以drive只能为0 */

    wini_identify(drive);

    if (intr_open != TRUE) panic("the hd int is not open\n", PANIC_ERR_NUM); /* 硬盘中断没打开，没得玩了 */

    /* 如果是第一次打开，这个磁盘，那么，得到其主分区信息 */
    if (hd_info[drive].open_count++ == 0) {
        partition(drive * (NR_PART_PER_DRIVE + 1), P_PRIMARY);
        kprintf("{HD}-> Reading partition information succeeded :)\n");
    }
    kprintf("<HD>-> open succeeded :)\n");
    return OK;
}

/**
 * 确定磁盘信息
 * @param drive  驱动器号
 * @return
 */
PRIVATE int wini_identify(int drive) {
    /**
     * 确定一个驱动器对应的磁盘，并得到磁盘信息，在最后，
     * 才真正初始化硬盘中断，因为我们准备要用它了。
     */
    Command cmd;
    cmd.device = MAKE_DEVICE_REG(0, drive, 0);
    cmd.command = ATA_IDENTIFY;
    cmd_out(&cmd);
    wini_interrupt_wait();  /* 现在等待硬盘响应一个中断 */
    kprintf("ready to get disk info\n");
    port_read(REG_DATA, hdbuf, SECTOR_SIZE);

    /* 打印通过ATA_IDENTIFY命令检索的hdinfo */
    wini_print_identify_info((u16_t *) hdbuf);

    u16_t *hdinfo = (u16_t *) hdbuf;

    hd_info[drive].primary[0].base = 0;
    /* 用户可寻址扇区的总数量 */
    hd_info[drive].primary[0].size = ((int) hdinfo[61] << 16) + hdinfo[60]; /* 硬盘总扇区数 */

    /* 现在可以启用中断了 */
    if (intr_open == FALSE) {
        put_irq_handler(AT_WINI_IRQ, wini_handler);
        enable_irq(CASCADE_IRQ);
        enable_irq(AT_WINI_IRQ);
        intr_open = TRUE;
    }
}

/**
 * 打印通过ATA_IDENTIFY命令检索的hdinfo
 * @param hdinfo
 */
PRIVATE void wini_print_identify_info(u16_t *hdinfo) {
    int i, k;
    char s[64];

    struct ident_info_ascii {
        int idx;
        int len;
        char *desc;
    } iinfo[] = {{10, 20, "HD SN"},     /* Serial number in ASCII */
                 {27, 40, "HD Model"}   /* Model number in ASCII */ };

    for (k = 0; k < sizeof(iinfo) / sizeof(iinfo[0]); k++) {
        char *p = (char *) &hdinfo[iinfo[k].idx];
        for (i = 0; i < iinfo[k].len / 2; i++) {
            s[i * 2 + 1] = *p++;
            s[i * 2] = *p++;
        }
        s[i * 2] = 0;
        kprintf("{HD}-> %s: %s\n", iinfo[k].desc, s);
    }

    int capabilities = hdinfo[49];
    kprintf("{HD}-> LBA supported: %s\n", (capabilities & 0x0200) ? "Yes" : "No");

    int cmd_set_supported = hdinfo[83];
    kprintf("{HD}-> LBA48 supported: %s\n", (cmd_set_supported & 0x0400) ? "Yes" : "No");

    int sectors = ((int) hdinfo[61] << 16) + hdinfo[60];
    kprintf("{HD}-> HD size: %dMB\n", sectors * 512 / 1000000);
}

/**
 * 读取分区信息
 * @param device 次设备号
 * @param style 读主分区还是扩展分区？
 */
PRIVATE void partition(int device, int style) {
    /* 第一次打开设备时将调用此例程。 它读取分区表并填充hd_info结构。 */
    int i, drive;
    drive = DRIVER_OF_DEVICE(device);
    HDInfo *hdi = &hd_info[drive];

    PartEntry part_tab[NR_SUB_PER_DRIVE];

    if (style == P_PRIMARY) {
        /* 查找主分区 */
        get_part_table(drive, drive, part_tab);

        int nr_prim_parts = 0;
        for (i = 0; i < NR_PART_PER_DRIVE; i++) { /* 0~3 */
            if (part_tab[i].sysind == NO_PART) {
                continue;
            }
            nr_prim_parts++;
            int dev_nr = i + 1;          /* 主分区包括扩展分区 分区号1~4 */
            hdi->primary[dev_nr].base = part_tab[i].lowsec;
            hdi->primary[dev_nr].size = part_tab[i].size;
            kprintf("primary: %d-{%d | %d}\n", dev_nr, hdi->primary[dev_nr].base, hdi->primary[dev_nr].size);

            if (part_tab[i].sysind != EXT_PART &&
                (largestPrimDeviceNR == 0 || hdi->primary[dev_nr].size > hdi->primary[largestPrimDeviceNR].size))
                largestPrimDeviceNR = dev_nr;

            if (part_tab[i].sysind == EXT_PART) {    /* 扩展分区？继续获取分区 */
                partition(device + dev_nr, P_EXTENDED);
            }
        }
        if (nr_prim_parts == 0) panic("nr_prim_parts can not be 0\n", PANIC_ERR_NUM);
    } else if (style == P_EXTENDED) {
        /* 查找扩展分区 */
        int j = device % NR_PRIM_PER_DRIVE;     /* 1~4 */
        int ext_start_sect = hdi->primary[j].base;
        int s = ext_start_sect;
        int nr_1st_sub = (j - 1) * NR_SUB_PER_PART; /* 0/16/32/48 */

        for (i = 0; i < NR_SUB_PER_PART; i++) {
            int dev_nr = nr_1st_sub + i;/* 0~15/16~31/32~47/48~63 */

            get_part_table(drive, s, part_tab);

            hdi->logical[dev_nr].base = s + part_tab[0].lowsec;
            hdi->logical[dev_nr].size = part_tab[0].size;

            s = ext_start_sect + part_tab[1].lowsec;
            kprintf("logical: %d-{%d | %d}\n", dev_nr, hdi->logical[dev_nr].base, hdi->logical[dev_nr].size);

            if (largestLogicDeviceNR == 0 || hdi->logical[dev_nr].size > hdi->logical[largestLogicDeviceNR].size)
                largestLogicDeviceNR = dev_nr;

            /* 此扩展分区中不再有更多逻辑分区 */
            if (part_tab[1].sysind == NO_PART) break;
        }
    } else {
        panic("hd panic\n", PANIC_ERR_NUM);
    }

}

/**
 * 获取分区表
 * @param drive 驱动器号（第一个磁盘为0，第二个磁盘为1，...）
 * @param sect_nr 分区表所在的扇区。
 * @param entry 指向一个分区
 */
PRIVATE void get_part_table(int drive, int sect_nr, PartEntry *entry) {//todo 有问题
    Command cmd;
    cmd.features = 0;
    cmd.count = 1;
    cmd.lba_low = sect_nr & 0xFF;
    cmd.lba_mid = (sect_nr >> 8) & 0xFF;
    cmd.lba_high = (sect_nr >> 16) & 0xFF;
    cmd.device = MAKE_DEVICE_REG(1, drive, (sect_nr >> 24) & 0xF); /* LBA模式 */
    cmd.command = ATA_READ;
    cmd_out(&cmd);
    wini_interrupt_wait(); /* 要防止丢失通知 */
    port_read(REG_DATA, hdbuf, SECTOR_SIZE);
    memcpy(entry, hdbuf + PARTITION_TABLE_OFFSET, sizeof(PartEntry) * NR_PART_PER_DRIVE);
}

/**
 * 关闭设备
 * @param device 设备号
 * @return
 */
PRIVATE int wini_do_close(int device) {
    int drive = DRIVER_OF_DEVICE(device);
    /* 现在只能处理一个硬盘，所以drive只能为0 */
    if (device != 0) panic("device only be 0\n", PANIC_ERR_NUM);

    hd_info[drive].open_count--;
    return OK;
}

/**
 * 硬盘IO控制，获取或设置某设备的分区信息
 * @param msg 请求消息
 * @return
 */
PRIVATE int wini_do_ioctl(Message *p_msg) {
    int device = p_msg->DEVICE;
    int drive = DRIVER_OF_DEVICE(device);

    /* 得到硬盘信息 */
    HDInfo *hp = &hd_info[drive];

    int request = p_msg->REQUEST;
    /* 请求不对 */
    if (request != DIOCTL_GET_GEO && request != DIOCTL_SET_GEO) return ENOTTY;

    Partition tmp_par;
    /* 得到用户缓冲区 */
    phys_addr user_phys = proc_vir2phys(proc_addr(p_msg->PROC_NR), (vir_addr) p_msg->ADDRESS);
    /* 得到现在硬盘分区信息 */
    Partition *par = device < MAX_PRIM ? &hp->primary[device] : &hp->logical[(device - MINOR_hd1a) % NR_SUB_PER_DRIVE];
    /* 处理请求 */
    if (request == DIOCTL_GET_GEO) {
        phys_addr par_phys = vir2phys(par);
        /* 复制给用户，OK */
        phys_copy(par_phys, user_phys, (phys_addr) sizeof(tmp_par));
    } else {    /* IOCTL_SET_GEO */
        /* 先将用户的分区信息复制过来 */
        phys_copy(user_phys, vir2phys(&tmp_par), (phys_addr) sizeof(tmp_par));
        /* 设置分区信息 */
        par->base = tmp_par.base;
        par->size = tmp_par.size;
    }
    return OK;
}

/**
 * 硬盘读写
 * @param msg
 * @return
 */
PRIVATE int wini_do_readwrite(Message *p_msg) {
    int drive = DRIVER_OF_DEVICE(p_msg->DEVICE);

    off_t pos = p_msg->POSITION;
//    assert((pos >> SECTOR_SIZE_SHIFT) < (1 << 31));
    if ((pos >> SECTOR_SIZE_SHIFT) >= (1 << 31)) panic("pos error\n", PANIC_ERR_NUM);

    /* 我们仅允许从扇区边界进行读/写： */
//    assert((pos & 0x1FF) == 0);
    if ((pos & 0x1FF) != 0) panic("pos error\n", PANIC_ERR_NUM);

    u32_t sect_nr = (u32_t) pos >> SECTOR_SIZE_SHIFT;  /* pos在分区内的扇区号 */
    int logidx = (p_msg->DEVICE - MINOR_hd1a) % NR_SUB_PER_DRIVE; /* 次设备号 不会出现10~15 */
    sect_nr +=
            p_msg->DEVICE < MAX_PRIM ? hd_info[drive].primary[p_msg->DEVICE].base : hd_info[drive].logical[logidx].base;

//    kprintf("%d want to %s %d by %d | pos -> %d\n",
//            p_msg->PROC_NR, p_msg->type == DEVICE_READ ? "read" : "write", p_msg->COUNT, drive, pos);

    /* 发出读/写命令，告诉驱动器开始读/写了。 */
    Command cmd;
    cmd.features = 0;
    cmd.count = (p_msg->COUNT + SECTOR_SIZE - 1) / SECTOR_SIZE;
    cmd.lba_low = sect_nr & 0xFF;
    cmd.lba_mid = (sect_nr >> 8) & 0xFF;
    cmd.lba_high = (sect_nr >> 16) & 0xFF;
    cmd.device = MAKE_DEVICE_REG(1, drive, (sect_nr >> 24) & 0xF);
    cmd.command = (p_msg->type == DEVICE_READ) ? ATA_READ : ATA_WRITE;
    if (cmd_out(&cmd) == OK) {
        /* 用户的缓冲区物理地址 */
        phys_addr addr = proc_vir2phys(proc_addr(p_msg->PROC_NR), (vir_addr) p_msg->ADDRESS);

        int left = p_msg->COUNT;
        /* 做真正的事情吧 */
        while (left) {
            /* 如果要读写的太多了，那么先取磁盘能读写的一个扇区大小 */
            int bytes = MIN(SECTOR_SIZE, left);
            if (p_msg->type == DEVICE_READ) {     /* 读 */
                wini_interrupt_wait();
                /* 将磁盘数据读到缓冲区中 */
                port_read(REG_DATA, hdbuf, SECTOR_SIZE);
                /* 拷贝给用户 */
                phys_copy(hdbuf_phys, addr, (phys_addr) bytes);
            } else {                        /* 写 */
                /* 等待控制器到可被写入的状态，如果超时，宕机 */
                if (!wini_wait_for(STATUS_DRQ, STATUS_DRQ)) {
                    panic("HD Controller is already dumb.", PANIC_ERR_NUM);
                }
                /* 将用户数据写到磁盘中 */
                port_write(REG_DATA, (void *) addr, bytes);
                wini_interrupt_wait();
            }
            /* 一轮完成了 */
            left -= SECTOR_SIZE;
            addr += SECTOR_SIZE;
        }
        return p_msg->COUNT - left;   /* 成功返回读写的字节总量 */
    }
    kprintf("hd io error\n");
    return EIO;
}

/**
 * 硬盘批量读写
 * @param msg
 * @return
 */
PRIVATE int wini_do_vreadwrite(Message *msg) {
    /* 本例程实现同时完成多个读写请求@TODO */
    return EINVAL;
}

/* 返回设备名称 */
PRIVATE char *wini_name(void) {
    return "AT_HD0";    /* 现在只有一个硬盘的情况 */
}

/**
 * 硬盘中断处理程序
 * @param irq 中断向量
 * @return
 */
PRIVATE int wini_handler(int irq) {
    /**
     * 当硬盘任务第一次被激活时，wini_identify把这个中断处理程序
     * 的地址送入中断描述表中。
     */

    /* 得到磁盘控制器的状态 */
    wini_status = in_byte(REG_STATUS);

    /* 模拟硬件中断，激活硬盘任务 */
//    interrupt(wini_task_nr);
    aos_unpark(HD_TASK);
    return ENABLE;      /* 返回ENABLE，使其再能发生AT硬盘中断 */
}

/**
 * 等待驱动任务完成一个中断
 */
PRIVATE void wini_interrupt_wait(void) {
    Message t_msg;

    if (intr_open) {
        /* 等待一条中断将其唤醒 */
//        receive(ANY, &t_msg);
        /**
         * 这里不应该使用rec-interrupt机制，因为这样会很容易lost-wakeup。
         * 例如在get_part_table中，刚完成指令cmd_out，硬盘中断就来了，即先执行了interrupt，
         * 然而hd任务还没有执行到这里，于是interrupt了个寂寞。
         * 当hd任务执行到这里的时候，却错过了interrupt的时机，永远地阻塞在这里了。
         * 所以这里（所有异步的地方）要防止lost-wakeup。
         */
        park();
    } else {
        /* 尚未给驱动任务分配中断，使用轮询 */
        (void) wini_wait_for(STATUS_BSY, 0);
    }
}

/**
 * 轮询控制器
 * @param mask 状态掩码
 * @param value 所需状态
 * @return
 */
PRIVATE int wini_wait_for(int mask, int value) {
    /**
     * 忙等待，当控制器忙的时候，一直等待到其可用为止，超时将返回代码0。
     * 这里，有一点需要注意：磁盘的超时时间被设置为了31.7s，而普通进程
     * 的cpu的运行时间是100ms，所以这些参数对于忙等待而言，是很长很长
     * 的一段时间，但是，这些数值是基于已公布的AT类计算机硬盘接口标准的，
     * 这些标准指出了磁盘旋转到一定速度所允许的最长时间是31s，当然实际
     * 上，这是在最坏情况下的规范，在大多数系统中仅仅在刚上电时或在很长
     * 时间不活动之后，才需要启动旋转加速。
     * 现在经常旋转的硬件设备，例如CD_ROM，都已经基本被淘汰了，所以这套
     * 处理超时的方法是没有什么大问题的。
     */

    Message msgBackUp;              /* 备份消息，因为在与fs通信时，还要和clock通信，防止fs的消息丢失 */
    msgBackUp = msg;

    /* 得到当前时间 */
    time_t now = get_uptime();
    /* 发呆时间（轮询的时间） */
    time_t daze = 0;
    do {
        /* 循环，轮流检测状态寄存器和时间，
		 * 不超时，将一直循环，不忙了，返回代码1。
         */
        wini_status = in_byte(REG_STATUS);
        if ((wini_status & mask) == value) {
            msg = msgBackUp;
            return TRUE;
        }
        daze = get_uptime() - now;
    } while (daze < HD_TIMEOUT);

    msg = msgBackUp;

    /* 好了，这个控制器哑了，都超时了还在忙。重置他并返回状态0 */
    return FALSE;
}

PRIVATE clock_t get_uptime() {
//    msg.source = HD_TASK;
//    msg.type = GET_UPTIME;
//    send_rec(CLOCK_TASK, &msg);
//    clock_t time=msg.CLOCK_TIME;
    clock_t time = clock_get_uptime();
    return time;
}


/**
 * 输出一个命令字到硬盘控制器
 * @param cmd
 * @return
 */
PRIVATE int cmd_out(Command *cmd) {

    /* 调用wini_wait_for，询问控制器忙不忙，如果在忙就等待，
     * 等待如果超时，系统无法继续。
     */
    if (!wini_wait_for(STATUS_BSY, 0)) {
        panic("%s: controller no response\n", PANIC_ERR_NUM);
        return -1;
    }

    /**
     * 安排一个唤醒呼叫闹钟，一些控制器是脆弱的。
     * 磁盘驱动器执行代码时，有时会失败或不能正常的返回一个出错代码。
     * 毕竟驱动器是机械设备，内部有可能发生各种机械故障。所以作为一项
     * 保险措施，要向时钟任务发送一条消息以安排一个对唤醒例程的调用。
     */
//    alarm_clock(WAKEUP, wini_timeout_handler);

    /* 激活中断允许（nIEN）位 */
    out_byte(REG_DEV_CTRL, 0);
    /* 向各种寄存器写入参数再向命令寄存器写入命令代码来发出一条命令 */
    out_byte(REG_FEATURES, cmd->features);
    out_byte(REG_NSECTOR, cmd->count);
    out_byte(REG_LBA_LOW, cmd->lba_low);
    out_byte(REG_LBA_MID, cmd->lba_mid);
    out_byte(REG_LBA_HIGH, cmd->lba_high);
    out_byte(REG_DEVICE, cmd->device);
    /* 可以发出命令了 */
    out_byte(REG_CMD, cmd->command);
    return OK;
}