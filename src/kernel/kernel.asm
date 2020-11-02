; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   导入和导出
; ----------------------------------------------------------------------------------------------------------------------
; 导入头文件
%include "asm_constant.inc"
; 导入函数
extern init_c                       ; 初始化一些事情，主要是改变gp_gdt，让它指向新的GDT
extern aos_main                     ; 内核主函数
extern exception_handler            ; 异常统一处理例程


; 导入变量
extern gp_gdt                       ; GDT指针
extern gp_idt                       ; IDT指针
extern g_tss                        ; 任务状态段

; 导出函数
global _start                       ; 导出_start程序开始符号，链接器需要它
global STACK_TOP
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   内核数据段
; ----------------------------------------------------------------------------------------------------------------------
section .data
bits 32     ; 指示编译器产生在32位模式下工作的代码
    nop
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   内核堆栈段
; BSS段通常是指用来存放程序中未初始化的或者初始化为0的全局变量和静态变量的一块内存区域。
; "RESB", "RESW", "RESD", "RESQ" and "REST"被设计用在模块的 BSS 段中：它们声明未初始化的存储空间
; ----------------------------------------------------------------------------------------------------------------------
section .bss
stackSpace      resb 2 * 1024       ; 2KB bss空间
STACK_TOP
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   内核代码段
; ----------------------------------------------------------------------------------------------------------------------
section .text
_start:
; ----------------------------------------------------------------------------------------------------------------------
; _start是习惯命名, 实际上，不使用这个命名也是可以的，虽然会得到ld命令的一个警告，但是不影响程序的正确性
; 注意! 在使用 C 代码的时候一定要保证 ds, es, ss 这几个段寄存器的值是一样的。
; 因为编译器有可能编译出使用它们的代码, 而编译器默认它们是一样的. 比如串拷贝操作会用到 ds 和 es。
; ----------------------------------------------------------------------------------------------------------------------
    ; 寄存器复位
    mov ax, ds              ; 此时的ds值为loader.asm中的数据段选择子
    mov es, ax
    mov fs, ax
    mov ss, ax              ; es = fs = ss = 内核数据段
    mov esp, STACK_TOP      ; 栈顶

    sgdt [gp_gdt]           ; 将GDT寄存器里的值保存到gp_gdt中
; ----------------------------------------------------------------------------------------------------------------------
; 调用init_c函数做一系列的初始化工作，如拷贝到GDT到新的地方（旧的位置将来会被覆盖），改变gdt指针指向新的地方。
; ----------------------------------------------------------------------------------------------------------------------
    call init_c
    lgdt [gp_gdt]           ; 使用新的GDT
    lidt [gp_idt]           ; 加载idt指针，在cstart函数中已经将idt_ptr指向新的中断表了

    ; 一个跳转指令，刷新描述符高速缓存
    jmp KERNEL_CS_SELECTOR:pre_c

pre_c:
    ; 加载任务状态段 TSS
    xor eax, eax
    mov ax, TSS_SELECTOR
    ltr ax

;    int 0
;    int 48

    ; 跳入C语言编写的主函数，在这之后我们内核的开发工作主要用C开发了
    ; 这一步，我们迎来了质的飞跃，汇编虽然好，只是不够骚！
    jmp aos_main

    ; 永远不可能到的真实
    jmp $
; ======================================================================================================================


