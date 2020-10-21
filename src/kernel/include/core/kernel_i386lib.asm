; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   导入和导出
; ----------------------------------------------------------------------------------------------------------------------
; 导入头文件
%include "asm_constant.inc"

; 导入变量
extern g_display_position         ; 当前显存的显示位置
extern exception_handler            ; 异常统一处理例程

; 导出函数
global low_print                ; 低特权级打印函数，只能支持打印ASCII码
global phys_copy                ; 通过物理地址拷贝内存


; 所有的异常处理入口
global divide_error
global debug_exception
global non_maskable_int
global break_point
global over_flow
global out_of_bounds
global invalid_opcode
global dev_not_available
global double_fault
global coop_proc_seg_oob
global invalid_tss
global segment_not_present
global stack_exception
global general_protection
global page_fault
global math_fault
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   phys_copy
; ----------------------------------------------------------------------------------------------------------------------
; PUBLIC void phys_copy(phys_bytes src, phys_bytes dest, phys_bytes size);
;* 将物理内存中任意处的一个数据块拷贝到任意的另外一处 *
;* 参数中的两个地址都是绝对地址，也就是地址 0 确实表示整个地址空间的第一个字节， *
;* 并且三个参数均为无符号int *
PC_ARGS     equ     16    ; 用于到达复制的参数堆栈的栈顶
align 16    ; 函数的开始地址16字节对齐
phys_copy:
    push esi
    push edi
    push es

    ; 获得所有参数
    mov esi, [esp + PC_ARGS]            ; src
    mov edi, [esp + PC_ARGS + 4]        ; dest
    mov ecx, [esp + PC_ARGS + 4 + 4]    ; size
    ; 注：因为得到的就是物理地址，所以esi和edi无需再转换，直接就表示一个真实的位置。
    mov eax, ecx
    and eax, 0x3                        ; 得到 size / 4 的余数，肯定在 0~3 内，用与是为了速度
    shr ecx, 2                          ; 得到 size / 4 的结果，因为我们要执行 dword 的传输
    cld
    rep movsd                           ; 双字传输，效率第一！
    mov ecx, eax                        ; 好的，现在准备传输剩下的字节
    cld
    rep movsb                           ; 字节传输剩下的 0~3 个字节

    pop es
    pop edi
    pop esi
    ret
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   打印函数，它类似与C语言中的printf，但它不支持'%'可变参数
; 函数原型：void low_print(char* str)，字符串以0结尾
; ----------------------------------------------------------------------------------------------------------------------
align 16    ; 函数地址16字节对齐
low_print:
    push esi
    push edi

    mov esi, [esp + 4 * 3]          ; 得到字符串地址
    mov edi, [g_display_position]     ; 得到显示位置
    mov ah, 0xf                     ; 黑底白字
.1:
    lodsb                           ; ds:esi -> al, esi++
    test al, al
    jz .print_end                   ; 遇到了0，结束打印
    cmp al, 10
    je .2
    ; 如果不是0，也不是'\n'，那么我们认为它是一个可打印的普通字符
    mov [gs:edi], ax
    add edi, 2                      ; 指向下一列
    jmp .1
.2: ; 处理换行符'\n'
    push eax
    mov eax, edi                    ; eax = 显示位置
    mov bl, 160
    div bl                          ; 显示位置 / 160，商eax就是当前所在行数
    inc eax                         ; 行数++
    mov bl, 160
    mul bl                          ; 行数 * 160，得出这行的显示位置
    mov edi, eax                    ; edi = 新的显示位置
    pop eax
    jmp .1
.print_end:
    mov dword [g_display_position], edi ; 打印完毕，更新显示位置

    pop edi
    pop esi
    ret
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   异常处理，只考虑i3860~16号，详细信息参照《自己动手写操作系统》p110
; ----------------------------------------------------------------------------------------------------------------------
divide_error:           ; 除法错误
    call save
	push	0xffffffff	; 没有错误代码，用0xffffffff表示
	push	0		    ; 中断向量号	= 0
	jmp	exception

debug_exception:        ; 调试错误
    call save
	push	0xffffffff	; 没有错误代码，用0xffffffff表示
	push	1		    ; 中断向量号	= 1
	jmp	exception

non_maskable_int:       ; 不可屏蔽中断
    call save
	push	0xffffffff	; 没有错误代码，用0xffffffff表示
	push	2		    ; 中断向量号	= 2
	jmp	exception

break_point:            ; 调试断点
    call save
	push	0xffffffff	; 没有错误代码，用0xffffffff表示
	push	3		    ; 中断向量号	= 3
	jmp	exception

over_flow:              ; 溢出
    call save
	push	0xffffffff	; 没有错误代码，用0xffffffff表示
	push	4		    ; 中断向量号	= 4
	jmp	exception

out_of_bounds:          ; 越界错误
    call save
	push	0xffffffff	; 没有错误代码，用0xffffffff表示
	push	5		    ; 中断向量号	= 5
	jmp	exception

invalid_opcode:         ; 无效操作码
    call save
	push	0xffffffff	; 没有错误代码，用0xffffffff表示
	push	6		    ; 中断向量号	= 6
	jmp	exception

dev_not_available:      ; 设备不可用
    call save
	push	0xffffffff	; 没有错误代码，用0xffffffff表示
	push	7		    ; 中断向量号	= 7
	jmp	exception

double_fault:           ; 双重错误
    call save
	push	8		    ; 中断向量号	= 8
	jmp	exception

coop_proc_seg_oob:      ; 协处理器段越界
    call save
	push	0xffffffff	; 没有错误代码，用0xffffffff表示
	push	9		    ; 中断向量号	= 9
	jmp	exception

invalid_tss:            ; 无效tss
    call save
	push	10		    ; 中断向量号	= 10
	jmp	exception

segment_not_present:    ; 段不存在
    call save
	push	11		    ; 中断向量号	= 11
	jmp	exception

stack_exception:        ; 堆栈段错误
    call save
	push	12		    ; 中断向量号	= 12
	jmp	exception

general_protection:     ; 常规保护错误
    call save
	push	13		    ; 中断向量号	= 13
	jmp	exception

page_fault:             ; 页错误
    call save
	push	14		    ; 中断向量号	= 14
	jmp	exception

                        ; 中断向量号15保留，未使用

math_fault:             ; 浮点数错误
    call save
	push	0xffffffff	; 没有错误代码，用0xffffffff表示
	push	16		    ; 中断向量号	= 16
	jmp	exception

exception:
	call	exception_handler
	add	esp, 4 * 2	    ; 让栈顶指向 EIP，堆栈中从顶向下依次是：EIP、CS、EFLAGS
    ret                 ; 系统已将异常解决，继续运行！
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   保护现场
; ----------------------------------------------------------------------------------------------------------------------
save:

; ======================================================================================================================