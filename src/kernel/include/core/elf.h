//
// Created by 杜科 on 2021/1/9.
//

#ifndef AOS_ELF_H
#define AOS_ELF_H

typedef u32_t Elf32_Word, Elf32_Addr, Elf32_Off;
typedef u16_t Elf32_Half;

typedef struct {
    /* 偏移：0 */
    unsigned char e_ident[16];    /* Magic number and other info */
    /* 偏移：16 */
    Elf32_Half e_type;                /* Object file type */
    /* 偏移：18 */
    Elf32_Half e_machine;            /* Architecture */
    /* 偏移：20 */
    Elf32_Word e_version;            /* Object file version */
    /* 偏移：24 */
    Elf32_Addr e_entry;            /* Entry point virtual address */
    /* 偏移：28 */
    Elf32_Off e_phoff;            /* Program header table file offset */
    /* 偏移：32 */
    Elf32_Off e_shoff;            /* Section header table file offset */
    /* 偏移：36 */
    Elf32_Word e_flags;            /* Processor-specific flags */
    /* 偏移：40 */
    Elf32_Half e_ehsize;            /* ELF header size in bytes */
    /* 偏移：42 */
    Elf32_Half e_phentsize;        /* Program header table entry size */
    /* 偏移：44 */
    Elf32_Half e_phnum;            /* Program header table entry count */
    /* 偏移：46 */
    Elf32_Half e_shentsize;        /* Section header table entry size */
    /* 偏移：48 */
    Elf32_Half e_shnum;            /* Section header table entry count */
    /* 偏移：50 */
    Elf32_Half e_shstrndx;            /* Section header string table index */
} Elf32_Ehdr;    /* 大小：52字节 */

/* program header程序表头 (段描述头) */
typedef struct {
    /* 偏移：0 */
    Elf32_Word p_type;            /* Segment type */
    /* 偏移：4 */
    Elf32_Off p_offset;        /* Segment file offset */
    /* 偏移：8 */
    Elf32_Addr p_vaddr;        /* Segment virtual address */
    /* 偏移：12 */
    Elf32_Addr p_paddr;        /* Segment physical address */
    /* 偏移：16 */
    Elf32_Word p_filesz;        /* Segment size in file */
    /* 偏移：20 */
    Elf32_Word p_memsz;        /* Segment size in memory */
    /* 偏移：24 */
    Elf32_Word p_flags;        /* Segment flags */
    /* 偏移：28 */
    Elf32_Word p_align;        /* Segment alignment */
} Elf32_Phdr;    /* 大小：32字节 */

/* section header节表头 */
typedef struct {
    Elf32_Word sh_name;
    Elf32_Word sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;
} Elf32_Shdr;

/* 定义elf魔数 */
// #define ELFMAG  0x464c457f
#define ELFMAG  1179403647
/* 定义elf mag长度 */
#define SELFMAG 4

/* sh_type字段定义 */
#define SHT_NULL        0
#define SHT_PROGBITS    1
#define SHT_SYMTAB      2
#define SHT_STRTAB      3
#define SHT_RELA        4
#define SHT_HASH        5
#define SHT_DYNAMIC     6
#define SHT_NOTE        7
#define SHT_NOBITS      8
#define SHT_REL         9
#define SHT_SHLIB       10
#define SHt_DYNSYM      11

/* sh_falgs字段定义 */
#define SHF_WRITE       0x1
#define SHF_ALLOC       0x2
#define SHF_EXECINSTR   0x4
#define SHF_MASKPROC    0xF0000000

#define PT_LOAD         1               /* Loadable program segment */
#define PT_DYNAMIC      2               /* Dynamic linking information */
#define PT_INTERP       3               /* Program interpreter */

#endif //AOS_ELF_H
