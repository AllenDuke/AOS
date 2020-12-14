; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   导入和导出
; ----------------------------------------------------------------------------------------------------------------------
; 导入头文件
%include "asm_constant.inc"

; 导入变量
extern g_dispPosition               ; 当前显存的显示位置
extern g_irqHandlers                ; 硬件中断请求处理例程表
extern gp_curProc                   ; 当前进程
extern g_tss
extern kernelReenter               ; 记录内核重入次数
extern STACK_TOP
extern level0Fn
extern sys_call                     ; 系统调用处理函数

; 导入函数
extern exception_handler            ; 异常统一处理例程
extern unhold                       ; 处理挂起的中断

; 导出函数
global low_print                    ; 低特权级打印函数，只能支持打印ASCII码
global phys_copy                    ; 通过物理地址拷贝内存
global cpu_halt
global in_byte                      ; 从一个端口读取一字节数据
global out_byte                     ; 向一个端口输出一字节数据
global in_word                      ; 从一个端口读取一字数据
global out_word                     ; 向一个端口输出一字数据
global interrupt_lock               ; 关闭中断响应，即锁中断
global interrupt_unlock             ; 打开中断响应，即解锁中断
global disable_irq                  ; 屏蔽一个特定的中断
global enable_irq                   ; 启用一个特定的中断
global restart
global level0
global level0_sys_call
global halt
global aos_sys_call
global msg_copy
global cmos_read                ; 从 CMOS 读取数据

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

; 所有中断处理入口，一共16个(两个8259A)
global	hwint00
global	hwint01
global	hwint02
global	hwint03
global	hwint04
global	hwint05
global	hwint06
global	hwint07
global	hwint08
global	hwint09
global	hwint10
global	hwint11
global	hwint12
global	hwint13
global	hwint14
global	hwint15
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   phys_copy
; ----------------------------------------------------------------------------------------------------------------------
; PUBLIC void phys_copy(phys_addr src, phys_addr dest, u32_t size);
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
    mov edi, [g_dispPosition]     ; 得到显示位置
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
    mov dword [g_dispPosition], edi ; 打印完毕，更新显示位置

    pop edi
    pop esi
    ret
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   异常处理，只考虑i3860~16号，详细信息参照《自己动手写操作系统》p110
; todo 使用c语言重写
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
;   系统进入宕机
; ----------------------------------------------------------------------------------------------------------------------
cpu_halt:
    hlt                 ; cpu进入待机状态，应该会降频以减少耗电，但可响应中断
    jmp cpu_halt
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   从一个端口读取一字节数据
; 函数原型： u8_t in_byte(port_t port)
; ----------------------------------------------------------------------------------------------------------------------
align 16
in_byte:
    push edx
    mov edx, [esp + 4 * 2]      ; 得到端口号
    xor eax, eax
    in al, dx                   ; port -> al
    nop                         ; 一点延迟
    pop edx
    nop
    ret
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   向一个端口输出一字节数据
; 函数原型： void out_byte(port_t port, U8_t value)
; ----------------------------------------------------------------------------------------------------------------------
align 16
out_byte:
    push edx
    mov edx, [esp + 4 * 2]      ; 得到端口号
    mov al, [esp + 4 * 3]   ; 要输出的字节
    out dx, al              ; al -> port
    nop                         ; 一点延迟
    pop edx
    nop
    ret
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   从一个端口读取一字数据
; 函数原型： u16_t in_word(port_t port)
; ----------------------------------------------------------------------------------------------------------------------
align 16
in_word:
    push edx
    mov edx, [esp + 4 * 2]      ; 得到端口号
    xor eax, eax
    in ax, dx              ; port -> ax
    pop edx
    nop                         ; 一点延迟
    ret
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   向一个端口输出一字数据
; 函数原型： void out_word(port_t port, U16_t value)
; ----------------------------------------------------------------------------------------------------------------------
align 16
out_word:
    push edx
    mov edx, [esp + 4 * 2]      ; 得到端口号
    mov ax, [esp + 4 * 3]   ; 得到要输出的变量
    out dx, ax              ; ax -> port
    pop edx
    nop                         ; 一点延迟
    ret
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   关闭中断响应，也称为锁中断
; 函数原型： void interrupt_lock(void)
; ----------------------------------------------------------------------------------------------------------------------
align 16
interrupt_lock:
    cli
    ret
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   打开中断响应，也称为解锁中断
; 函数原型： void interrupt_unlock(void)
; ----------------------------------------------------------------------------------------------------------------------
align 16
interrupt_unlock:
    sti
    ret
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   屏蔽一个特定的中断
; 函数原型： int disable_irq(int int_request);
; ----------------------------------------------------------------------------------------------------------------------
align 16
disable_irq:
    pushf                   ; 将标志寄存器 EFLAGS 压入堆栈，需要用到test指令，会改变 EFLAGS
    push ecx

    cli                     ; 先屏蔽所有中断
    mov ecx, [esp + 4 * 3]  ; ecx = int_request(中断向量)
    ; 判断要关闭的中断来自于哪个 8259A
    mov ah, 1               ; ah = 00000001b
    rol ah, cl              ; ah = (1 << (int_request % 8))，算出在int_request位的置位位图，例如2的置位位图是00000100b
    cmp cl, 7
    ja disable_slave        ; 0~7主，8~15从；> 7是从，跳转到 disable_slave 处理 从8259A 的中断关闭
disable_master:                 ; <= 7是主
    in al, INT_M_CTL_MASK    ; 取出 主8259A 当前的屏蔽位图
    test al, ah
    jnz disable_already     ; 该int_request的屏蔽位图不为0，说明已经被屏蔽了，没必要继续了
    ; 该int_request的屏蔽位图为0，还未被屏蔽
    or al, ah               ; 将该中断的屏蔽位置位，表示屏蔽它
    out INT_M_CTL_MASK, al   ; 输出新的屏蔽位图，屏蔽该中断
    jmp disable_ok          ; 屏蔽完成
disable_slave:
    in al, INT_S_CTL_MASK    ; 取出 从8259A 当前的屏蔽位图
    test al, ah
    jnz disable_already     ; 该int_request的屏蔽位图不为0，说明已经被屏蔽了，没必要继续了
    ; 该int_request的屏蔽位图为0，还未被屏蔽
    or al, ah               ; 将该中断的屏蔽位置位，表示屏蔽它
    out INT_S_CTL_MASK, al   ; 输出新的屏蔽位图，屏蔽该中断
disable_ok:
    pop ecx
    popf
    and eax, 1              ; 等同于 mov eax, 1，即return 1；我只是想耍个帅！
    ret
disable_already:
    pop ecx
    popf                    ; 恢复标志寄存器
    xor eax, eax            ; return 0，表示屏蔽失败，因为该中断已经处于屏蔽状态
    ret
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   启用一个特定的中断
; 函数原型： void enable_irq(int int_request);
; ----------------------------------------------------------------------------------------------------------------------
align 16
enable_irq:
    pushf                   ; 将标志寄存器 EFLAGS 压入堆栈，需要用到test指令，会改变 EFLAGS
    push ecx

    cli                     ; 先屏蔽所有中断
    mov ecx, [esp + 4 * 3]  ; ecx = int_request(中断向量)
    mov ah, ~1              ; ah = 11111110b
    rol ah, cl              ; ah = ~(1 << (int_request % 8))，算出在int_request位的复位位位图，例如2的置位位图是11111011b
    cmp cl, 7
    ja enable_slave         ; 0~7主，8~15从；> 7是从，跳转到 disable_slave 处理 从8259A 的中断关闭
enable_master:                  ; <= 7是主
    in al, INT_M_CTL_MASK    ; 取出 主8259A 当前的屏蔽位图
    and al, ah              ; 将该中断的屏蔽位复位，表示启用它
    out INT_M_CTL_MASK, al   ; 输出新的屏蔽位图，启用该中断
    jmp enable_ok
enable_slave:
    in al, INT_S_CTL_MASK    ; 取出 从8259A 当前的屏蔽位图
    and al, ah              ; 将该中断的屏蔽位复位，表示启用它
    out INT_S_CTL_MASK, al   ; 输出新的屏蔽位图，启用该中断
enable_ok:
    pop ecx
    popf
    ret
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   硬件中断处理
; 为 主从两个8259A 各定义一个中断处理模板
; 这里用宏而不是用函数，其实是牺牲了空间来获取时间，使得宏在每一个调用处展开，对比函数调用，占了更大的空间，但是运行速度比函数调用快，
; 因为函数调用，需要一系列的操作，而中断是要求速度的。
; 注意中断发生时，堆栈的变化
; ----------------------------------------------------------------------------------------------------------------------
%macro  hwint_master 1
    ; 0 为了支持多进程，发生中断，先保存之前运行进程的状态信息
    call save

    ; 1 在调用对于中断的处理例程前，先屏蔽当前中断，防止短时间内连续发生好几次同样的中断
    in al, INT_M_CTL_MASK    ; 取出 主8259A 当前的屏蔽位图
    or al, (1 << %1)        ; 将该中断的屏蔽位置位，表示屏蔽它
    out INT_M_CTL_MASK, al   ; 输出新的屏蔽位图，屏蔽该中断

    ; 2 重新启用 主8259A 和中断响应；CPU 响应一个中断后会自动关闭中断，同时 8259A 也会自动关闭自己
    ;   重新启用中断响应是为了及时的响应其他中断，很简单的道理：你在敲键盘的时候？就不允许磁盘中断响应操作磁盘了么？
    mov al, EOI
    out INT_M_CTL, al       ; 设置 EOI 位，重新启用 主8259A
    nop
    sti                     ; 重新启动中断响应，允许其他中断嵌套

    ; 3 现在调用中断处理例程
    push %1                 ; 压入中断向量号作为参数
    call [g_irqHandlers + (4 * %1)] ; 调用中断处理程序表中的相应处理例程，返回值存放在 eax 中
    add esp, 4              ; 清理堆栈

    ; 4 最后，判断用户的返回值，如果是DISABLE(0)，我们就直接结束；如果不为0，那么我们就重新启用当前中断
    cli                     ; 先将中断响应关闭，这个时候不允许其它中断的干扰
    cmp eax, DISABLE
    je .0                   ; 返回值 == DISABLE，直接结束中断处理
    ; 返回值 != DISABLE，重新启用当前中断
    in al, INT_M_CTL_MASK    ; 取出 主8259A 当前的屏蔽位图
    and al, ~(1 << %1)      ; 将该中断的屏蔽位复位，表示启用它
    out INT_M_CTL_MASK, al   ; 输出新的屏蔽位图，启用该中断
.0:
    ; 这个 ret 指令将会跳转到我们 save 中手动保存的地址，restart 或 restart_reenter
    ; 这里不使用sti，因为最后中断返回时使用了iret，最后也恢复了if位
;    sti                     ; 允许中断
    ret
%endmacro

align	16
hwint00:		; Interrupt routine for irq 0 (the clock)，时钟中断
 	hwint_master	0

align	16
hwint01:		; Interrupt routine for irq 1 (keyboard)，键盘中断
 	hwint_master	1

align	16
hwint02:		; Interrupt routine for irq 2 (cascade!)
 	hwint_master	2

align	16
hwint03:		; Interrupt routine for irq 3 (second serial)
 	hwint_master	3

align	16
hwint04:		; Interrupt routine for irq 4 (first serial)
 	hwint_master	4

align	16
hwint05:		; Interrupt routine for irq 5 (XT winchester)
 	hwint_master	5

align	16
hwint06:		; Interrupt routine for irq 6 (floppy)，软盘中断
 	hwint_master	6

align	16
hwint07:		; Interrupt routine for irq 7 (printer)，打印机中断
 	hwint_master	7

%macro  hwint_slave 1
    ; 0 为了支持多进程，发生中断，先保存之前运行进程的状态信息
    call save

    ; 1 在调用对于中断的处理例程前，先屏蔽当前中断，防止短时间内连续发生好几次同样的中断
    in al, INT_M_CTL_MASK    ; 取出 主8259A 当前的屏蔽位图
    or al, (1 << (%1 - 8)) ; 将该中断的屏蔽位置位，表示屏蔽它
    out INT_M_CTL_MASK, al   ; 输出新的屏蔽位图，屏蔽该中断

    ; 2 重新启用 主从8259A 和中断响应；因为 从8259A 的中断会级联导致 主8259A也被关闭，所以需要两个都重新启用
    mov al, EOI
    out INT_M_CTL, al       ; 设置 EOI 位，重新启用 主8259A
    nop
    out INT_S_CTL, al       ; 设置 EOI 位，重新启用 从8259A
    sti                     ; 重新启动中断响应

    ; 3 现在调用中断处理例程
    push %1                 ; 压入中断向量号作为参数
    call [g_irqHandlers + (4 * %1)] ; 调用中断处理程序表中的相应处理例程，返回值存放在 eax 中
    add esp, 4              ; 清理堆栈

    ; 4 最后，判断用户的返回值，如果是DISABLE(0)，我们就直接结束；如果不为0，那么我们就重新启用当前中断
    cli                     ; 先将中断响应关闭，这个时候不允许其它中断的干扰
    cmp eax, DISABLE
    je .0                   ; 返回值 == DISABLE，直接结束中断处理
    ; 返回值 != DISABLE，重新启用当前中断
    in al, INT_M_CTL_MASK    ; 取出 主8259A 当前的屏蔽位图
    and al, ~(1 <<(%1 - 8))      ; 将该中断的屏蔽位复位，表示启用它
    out INT_M_CTL_MASK, al   ; 输出新的屏蔽位图，启用该中断
.0:
    ; 这个 ret 指令将会跳转到我们 save 中手动保存的地址，restart 或 restart_reenter
    ret
%endmacro

align	16
hwint08:		; Interrupt routine for irq 8 (realtime clock).
 	hwint_slave	8

align	16
hwint09:		; Interrupt routine for irq 9 (irq 2 redirected)
 	hwint_slave	9

align	16
hwint10:		; Interrupt routine for irq 10
 	hwint_slave	10

align	16
hwint11:		; Interrupt routine for irq 11
 	hwint_slave	11

align	16
hwint12:		; Interrupt routine for irq 12
 	hwint_slave	12

align	16
hwint13:		; Interrupt routine for irq 13 (FPU exception)
 	hwint_slave	13

align	16
hwint14:		; Interrupt routine for irq 14 (AT winchester)
 	hwint_slave	14

align	16
hwint15:		; Interrupt routine for irq 15
 	hwint_slave	15
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   保护现场，把寄存器等相关信息保存到该进程的栈帧当中，根据是否中断嵌套设置中断返回地址
; ----------------------------------------------------------------------------------------------------------------------
save:
    ; 将所有的32位通用寄存器压入堆栈
    pushad
    ; 然后是特殊段寄存器
    push ds
    push es
    push fs
    push gs
    ; 注意：以上的操作都是在操作进程自己的堆栈

    ; ss 是内核数据段，设置 ds 和 es
    mov dx, ss
    mov ds, dx
    mov es, dx
    mov esi, esp                        ; esi 指向进程的栈帧开始处

    ; 相当于add [kernelReenter], 1，但inc速度快，占用空间小
    inc byte [kernelReenter]           ; 发生了一次中断，中断重入计数++
    ; 判断是不是嵌套中断，是的话就无需切换堆栈到内核栈了
    cmp	byte [kernelReenter], 0
    jnz .reenter                        ; 嵌套中断
    ; 从一个进程进入的中断，需要切换到内核栈
    mov esp, STACK_TOP
    push restart                        ; 压入 restart 例程，以便一会中断处理完毕后恢复，注意这里压入到的是内核堆栈
    jmp [esi + RETADDR - P_STACKBASE]   ; 回到 call save() 之后继续处理中断
.reenter:
    ; 嵌套中断，已经处于内核栈，无需切换
    push restart_reenter                ; 压入 restart_reenter 地址，以便一会中断处理完毕后恢复，注意这里压入到的是内核堆栈
    jmp [esi + RETADDR - P_STACKBASE]   ; 回到 call save() 之后继续处理中断
; ======================================================================================================================

;============================================================================
;   flyanx的系统调用
; 函数原型：void flyanx_386_sys_call(void);
; 系统调用流程：进程A进行系统调用（发送或接收一条消息），系统调用完成后，CPU控制权还是回到进程A手中，除非被调度程序调度。
;----------------------------------------------------------------------------
aos_sys_call:
    ; 这一部分是 save 函数的精简版，为了效率；但其实直接 call save 也没有任何问题
    push dword 0x3ea      ; 压入一个假的 ret_addr
    pushad
    push ds
    push es
    push fs
    push gs
    mov dx, ss
    mov ds, dx
    mov es, dx
    mov esi, esp                ; esi 指向进程的栈帧开始处
    inc byte [kernelReenter]  ; 发生了一次中断，中断重入计数++
    mov esp, STACK_TOP
    ; 重新开启中断,注：软件中断与硬件中断的相似之处还包括它们都会自动关中断
    sti
    ; 进行调用前，先保存进程的栈帧开始地址
    push esi

    ; 这里是重点，将所需参数压入栈中（现在处于核心栈），然后调用 sys_call 去真正调用实际的系统调用例程
    push ebx        ; msg_ptr
    push eax        ; src_dest_msgp
    push ecx        ; op
    call sys_call
    add esp, 4 * 3  ; clean

    ; 在完成进程恢复之前，关闭中断以保护即将被再次启动的进程的栈帧结构
    pop esi
    mov [esi + EAXREG - P_STACKBASE], eax   ; 将返回值放到调用进程的栈帧结构中的 eax 中
    cli
; 在这里，直接陷入 restart 的代码以重新启动进程/任务运行，这就是我们不需要完整 save 的原因
; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   中断处理完毕，恢复gp_curProc指向的进程
; 注意此时esp指向内核栈
; ----------------------------------------------------------------------------------------------------------------------
restart:
    ; 如果检测到存在被挂起的中断，这些中断是在处理其他中断期间到达的，
    ; 则调用 unhold，这样就允许在任何进程被重新启动之前将所有挂起的中断转换为消息。
    ; 这将暂时地重新关闭中断，但在 unhold 返回之前将再次打开中断。
    call unhold
over_unhold:
	mov esp, [gp_curProc]	            ; 离开内核栈，指向运行进程的栈帧，现在的位置是 gs
	lldt [esp + P_LDT_SEL]	            ; 每个进程有自己的 LDT，所以每次进程的切换都需要加载新的ldtr
	; 把该进程栈帧栈顶地址保存到 tss.ss0 中，方便下次中断时的 save 将保存所有寄存器到该进程的栈帧中
	lea eax, [esp + P_STACKTOP]         ; 把栈帧的栈顶物理地址存入eax
	mov dword [g_tss + TSS3_S_SP0], eax
restart_reenter:
	; 在这一点上，kernelReenter 被减1，因为一个中断已经处理完毕
	dec byte [kernelReenter]           ; 第一次调用restart后，kernelReenter为-1
	; 将该进程的栈帧中的所有寄存器信息恢复
    pop gs
    pop fs
    pop es
    pop ds
	popad
	; 跳过压入的 save 的返回地址
	add esp, 4
	; 中断返回：恢复中断时压入的eip cs eflags esp ss
	iretd                               ; 在首次调用时，也就开启了中断
; ======================================================================================================================


;============================================================================
;                  CPU 待机
;----------------------------------------------------------------------------
; 本 halt 例程使用指令 hlt 让 CPU 闲置进入待机状态
halt:
    sti                 ; 开中断，为了在待机下快速响应用户事件
    hlt                 ; 处理器休眠，中断可以再次打断该状态
    cli                 ; 关中断
    ret
;============================================================================
;   系统提权调用，只能给系统任务使用
; 函数原型：void level0_sys_call(void);
;----------------------------------------------------------------------------
level0_sys_call:
    call save
    jmp [level0Fn]       ; 好的，提权成功，我们现在已经处于内核代码段，直接跳转到需要提权的函数执行它
    ret

;============================================================================
;   将一个函数提权到 0，再进行调用
; 函数原型： void level0(aos_syscall_t func);
;----------------------------------------------------------------------------
align 16
level0:
    mov eax, [esp + 4]
    mov [level0Fn], eax  ; 将提权函数指针放到 level0_func 中
    int LEVEL0_VECTOR	    ; 好的，调用提权调用去执行提权成功的例程
    ret


;*===========================================================================*
;*				消息拷贝				     *
; 函数原型：PUBLIC void msg_copy(phys_bytes msg_phys, phys_bytes dest_phys);
; 虽然我们可以用 phys_copy 来进行消息的拷贝完成消息的传递，但是这样按字节传输的效率
; 对于我们的邮局来说效率太低了，这个函数专门用于消息拷贝，它使用了 movsd(拷贝单位: dword)
; 这样在 32位 模式下更快的传输指令，它能很大提升我们邮局的传信效率，但比起函数调用还是很慢。
;*===========================================================================*
align 16
msg_copy:
    push esi
    push edi
    push ecx

      mov esi, [esp + 4 * 4]  ; msg_phys
      mov edi, [esp + 4 * 5]  ; dest_phys

      ; 开始拷贝消息
      cld
      mov ecx, MESSAGE_SIZE   ; 消息大小(dword)
      rep movsd

    pop ecx
    pop edi
    pop esi
    ret

;============================================================================
;   从 CMOS 读取数据
; 函数原型： u8_t cmos_read(u8_t addr);
;----------------------------------------------------------------------------
cmos_read:
    push edx
        mov al, [esp + 4 * 2]   ; 要输出的字节
        out CLK_ELE, al         ; al -> CMOS ELE port
        nop                     ; 一点延迟
        xor eax, eax
        in al, CLK_IO           ; port -> al
        nop                     ; 一点延迟
    pop edx
    ret
