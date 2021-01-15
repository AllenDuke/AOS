//
// Created by 杜科 on 2021/1/14.
//

#include "core/kernel.h"
#include "core/elf.h"

PUBLIC int get_kernel_map(phys_addr *base, phys_addr *limit){
    /* 解析内核文件，获取内核映像的内存范围。 */

    /* 得到内核文件的elf32文件头 */
    Elf32_Ehdr *elf_header = (Elf32_Ehdr*)(gp_bootParam->kernelFileAddr);

    if(!(elf_header->e_ident[0]==127&&elf_header->e_ident[1]==69&&
            elf_header->e_ident[2]==76&&elf_header->e_ident[3]==70))
        panic("kernel file type err, it must be elf32\n",PANIC_ERR_NUM);

    /* 内核文件必须为ELF32格式 */
//    if(memcmp(elf_header->e_ident, ELFMAG, SELFMAG) != 0){
//        return ERROR_BAD_ELF;
//    }

    *base = ~0;
    phys_addr t = 0;
    int i;
    for(i = 0; i < elf_header->e_shnum; i++){
        Elf32_Shdr *section_header = (Elf32_Shdr*)(gp_bootParam->kernelFileAddr+
                                                   elf_header->e_shoff + i * elf_header->e_shentsize);
        if(section_header->sh_flags & SHF_ALLOC) {
            int bottom = section_header->sh_addr;
            int top = section_header->sh_addr + section_header->sh_size;

            if(*base > bottom) *base = bottom;
            if(t < top) t = top;
        }
    }
    if(*base >= t) {
        return ERROR_BAD_ELF;
    }
    *limit = t - *base - 1;

    return OK;
}