//
// Created by 杜科 on 2021/1/8.
//

#include "core/kernel.h"
#include "core/elf.h"
#include "stdio.h"

extern Message mm_msg;
extern u8_t *mmbuf;
extern const int MMBUF_SIZE;

/*****************************************************************************
 *                                do_exec
 *****************************************************************************/
/**
 * Perform the exec() system call.
 *
 * @return  Zero if successful, otherwise -1.
 *****************************************************************************/
PUBLIC int mm_do_exec() {
    /* get parameters from the message */
    int name_len = mm_msg.NAME_LEN;    /* length of filename */
    int src = mm_msg.source;    /* caller proc nr. */
    assert(name_len < MAX_PATH);
//    if (name_len >= MAX_PATH) panic("name is too long\n", name_len);

    char pathname[MAX_PATH];
    phys_copy((void *) proc_vir2phys(proc_addr(src), mm_msg.PATHNAME),
              (void *) proc_vir2phys(proc_addr(MM_TASK), pathname),
              name_len);
    pathname[name_len] = 0;    /* terminate the string */

    /* get the file size */
    struct stat s;
    int ret = stat(pathname, &s);
    if (ret != 0) {
        kprintf("{MM} MM::do_exec()::stat() returns error. %s", pathname);
        return -1;
    }

    /* read the file */
    int fd = open(pathname, O_RDWR);
    if (fd == -1)
        return -1;
    assert(s.st_size < MMBUF_SIZE);
//    if (s.st_size >= MMBUF_SIZE) panic("st_size is too large\n", s.st_size);
    read(fd, mmbuf, s.st_size);
    close(fd);

    /* overwrite the current proc image with the new one */
    Elf32_Ehdr *elf_hdr = (Elf32_Ehdr *) (mmbuf);
    int i;
    for (i = 0; i < elf_hdr->e_phnum; i++) {
        Elf32_Phdr *prog_hdr = (Elf32_Phdr *) (mmbuf + elf_hdr->e_phoff +
                                               (i * elf_hdr->e_phentsize));
        if (prog_hdr->p_type == PT_LOAD) {
            assert(prog_hdr->p_vaddr + prog_hdr->p_memsz <PROC_IMAGE_SIZE_DEFAULT);
//            if (prog_hdr->p_vaddr + prog_hdr->p_memsz >= PROC_IMAGE_SIZE_DEFAULT)
//                panic("img is too large\n", prog_hdr->p_vaddr + prog_hdr->p_memsz);
            phys_copy((void *) proc_vir2phys(proc_addr(MM_TASK), mmbuf + prog_hdr->p_offset),
                      (void *) proc_vir2phys(proc_addr(src), (void *) prog_hdr->p_vaddr),
                      prog_hdr->p_filesz);
        }
    }

    /* setup the arg stack */
    int orig_stack_len = mm_msg.BUF_LEN;
    char stackcopy[PROC_ORIGIN_STACK];
    phys_copy((void *) proc_vir2phys(proc_addr(src), mm_msg.BUF),
              (void *) proc_vir2phys(proc_addr(MM_TASK), stackcopy),
              orig_stack_len);

    u8_t *orig_stack = (u8_t *) (PROC_IMAGE_SIZE_DEFAULT - PROC_ORIGIN_STACK);

    int delta = (int) orig_stack - (int) mm_msg.BUF;

    int argc = 0;
    if (orig_stack_len) {    /* has args */
        char **q = (char **) stackcopy;
        for (; *q != 0; q++, argc++)
            *q += delta;
    }

    phys_copy((void *) proc_vir2phys(proc_addr(MM_TASK), stackcopy),
              (void *) proc_vir2phys(proc_addr(src), orig_stack),
              orig_stack_len);

    g_procs[src].regs.ecx = argc; /* argc */
    g_procs[src].regs.eax = (u32_t) orig_stack; /* argv */

    /* setup eip & esp */
    g_procs[src].regs.eip = elf_hdr->e_entry; /* @see _start.asm */
    g_procs[src].regs.esp = PROC_IMAGE_SIZE_DEFAULT - PROC_ORIGIN_STACK;

    strcpy(g_procs[src].name, pathname);

    return 0;
}
