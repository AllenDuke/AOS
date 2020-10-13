; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
; 字符串显示函数
; 显示一个字符串, 函数开始时 dh 中应该是字符串序号(0-based)，即 dh=0 -> "Booting..." dh=1 -> "Loaded ^-^"，且dh为要打印的行
; ----------------------------------------------------------------------------------------------------------------------
print_string:
    push ax
    push bp
    push ds
    push es
    push cx
    push bx
    push dx

    mov	ax, MESSAGE_LENGTH
    mul	dh
    add	ax, loaderMessage
    mov	bp, ax  ; ┓
    mov	ax, ds  ; ┣ ES:BP = 串地址
    mov	es, ax  ; ┛
    ; es = ds, 但由于两个段寄存器间没有直接通路，所以不能使用mov指令，也就是说mov指令的两个操作数不能同时为段寄存器
    ; 所以要借助第三方中转，比如可以  mov ax,ds   mov es,ax

    mov	cx, MESSAGE_LENGTH   ; CX = 串长度
    mov	ax, 1301h           ; AH = 13,  AL = 01h
    mov	bx, 0007h           ; 页号为0(BH = 0) 黑底白字(BL = 07h)
    mov	dl, 0               ; 要打印的位置 列
    int	10h

    pop dx
    pop bx
    pop cx
    pop es
    pop ds
    pop bp
    pop ax
    ret
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
; 作用: 从第 ax 个 Sector 开始, 将 cl 个 Sector 读入 es:bx 中
; 1.44MB的软盘的组成：2*80*18*512
;   1. 0 1 两个磁头
;   2. 每面 80 磁道
;   3. 每磁道 18 扇区
;   4. 每扇区 512 字节
; 所有扇区由0开始编号，即0~159，那么组成为(柱面号，磁头号，所处磁道扇区号)
; 设扇区号为 x
;                          ┌ 柱面号 = y >> 1
;       x           ┌ 商 y ┤
; -------------- => ┤      └ 磁头号 = y & 1
;  每磁道扇区数      │
;                   └ 余 z => 起始扇区号 = z + 1，实际上在硬件的编码中，是从1起的。
; ----------------------------------------------------------------------------------------------------------------------
read_sector:
    push	bp
    mov	bp, sp
    sub	esp, 2			    ; 辟出两个字节的堆栈区域保存要读的扇区数: byte [bp-2]

    mov	byte [bp-2], cl     ; 基址变址寻址，ss+bp-2
    push	bx			    ; 保存 bx
    mov	bl, [bpbSecPerTrk]	; bl: 除数，bpbSecPerTrk：每磁道扇区数，直接寻址，ds+bpbSecPerTrk，bl=18

    ; 除法指令，ax为被除数，商为y，余数为z
    div	bl		       	    ; y 在 al 中, z 在 ah 中
    inc	ah			        ; z ++
    mov	cl, ah			    ; cl <- 起始扇区号
    mov	dh, al			    ; dh <- y
    shr	al, 1			    ; y >> 1 (其实是 y/BPB_NumHeads, 这里BPB_NumHeads=2)
    mov	ch, al			    ; ch <- 柱面号
    and	dh, 1			    ; dh & 1 = 磁头号
    pop	bx			        ; 恢复 bx
    ; 至此, "柱面号，磁头号，所处磁道扇区号" 全部得到 ^^^^^^^^^^^^^^^^^^^^^^^^
    mov	dl, [bsDrvNum]		; 驱动器号 (0 表示 A 盘)

go_on_reading:
    mov	ah, 2				; 读
    mov	al, byte [bp-2]		; 读 al 个扇区
    int	13h                 ; 13h中断读磁盘
    jc	go_on_reading		; 如果读取错误 CF 会被置为 1, 这时就不停地读, 直到正确为止

    add	esp, 2
    pop	bp

    ret
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
; 作用：找到簇号为 ax 在 FAT 中的条目，然后将结果放入 ax 中。代入簇号2去分析。
; 注意：中间我们需要加载 FAT表的扇区到es:bx处，所以我们需要先保存es:bx。
; 要点是，找出簇号为 ax 所在的 FAT项 相对于 FAT表1 的字节偏移量，然后算出所在扇区 和 扇区内偏移量，将两个扇区读进内存，
; 最后能算出 该FAT项 的在内存的地址。这时要区分簇号是奇还是偶，将12位的值存到 ax 。
; ----------------------------------------------------------------------------------------------------------------------
get_fat_entry:
    push es
    push bx

    ; 在加载的段地址处开辟出新的空间用于存放加载的FAT表
    push ax
    mov ax, LOADER_SEG - 0x100
    mov es, ax
    pop ax

    ; 首先计算出簇号在FAT中的字节偏移量，然后还需要计算出该簇号的奇偶性、
    ; 偏移值: 簇号 * 3 / 2 的商，因为3个字节表示2个簇，所以字节和簇之间的比例就是3:2。
    mov byte [isOdd], 0     ; isOdd = FALSE
    mov bx, 3               ; bx = 3
    mul bx                  ; ax * 3 --> dx存放高16位，ax存放低16位
    mov bx, 2               ; bx = 2
    div bx                  ; dx:ax / 2 --> ax存放商，dx存放余数。
    cmp dx, 0
    je even
    mov byte [isOdd], 1     ; isOdd = TRUE
; ----------------------------------------------------------------------------------------------------------------------
; 此时ax为该FAT项相对于FAT表1的字节偏移量
; ----------------------------------------------------------------------------------------------------------------------
even:                       ; 偶数，意味者该FAT项的高4位在下一字节的低4位，反之意为着该FAT项的低4位在上一字节的高4位
    ; FAT表占 9个扇区 ， 字节偏移量 5 ， 5 / 512 -- 0 .. 5， FAT表中的0扇区， FAT表0扇区中这个簇号所在偏移是5
    ; 570   570 / 512 -- 1 .. 58， FAT表中的1扇区， FAT表1扇区中这个簇号所在偏移是58
    xor dx, dx              ; dx = 0
    mov bx, [bpbBytsPerSec] ; bx = 每扇区字节数
    div bx                  ; dx:ax / 每扇区字节数，ax(商)存放FAT项相对于FAT表中的扇区号，
    ; dx(余数)FAT项在相对于FAT表中的扇区的偏移。
    push dx                 ; 保存FAT项在相对于FAT表中的扇区的偏移。
    mov bx, 0               ; bx = 0，es:bx --> (LOADER_SEG - 0x100):0

    ; 其实编码上会有点奇怪的，因为有些认为是需要计算的，有些认为它一般是一个固定值，比如这里认为它位于第一个FAT表中
    add ax, SECTOR_NUM_OF_FAT1  ; 此句执行之后的 ax 就是 FATEntry 所在的扇区号，即加上FAT项的整体偏移量
    mov cl, 2               ; 读取两个扇区
    call read_sector        ; 一次读两个，避免发生边界错误问题，因为一个FAT项可能会跨越两个扇区
    pop dx                  ; 恢复FAT项在相对于FAT表中的扇区的偏移。
; ----------------------------------------------------------------------------------------------------------------------
; 此时dx为该FAT项所在扇区的字节偏移量，而扇区已被读取到内存es:bx处。
;        LOADER_SEG         |————| 往下的4KB空间足够存放该FAT项所在的中两个扇区内容。
;                           |    | 4kB？注意这里是段地址，寻址时会左移4位，1000h->4k
;        LOADER_SEG-100h    |————| es:bx
; ----------------------------------------------------------------------------------------------------------------------
    ; todo 这里的bx=0，应该可以换成 mov bx, dx
    add bx, dx              ; bx += FAT项在相对于FAT表中的扇区的偏移，得到FAT项在内存中的偏移地址，因为已经将扇区读取到内存中
    mov ax, [es:bx]         ; ax = 簇号对应的FAT项，但还没完成，读了两个字节
    cmp byte [isOdd], 1
    jne even_2
    ; 奇数FAT项处理
    shr ax, 4               ; 需要将低四位清零（他是上一个FAT项的高四位）
    jmp get_fat_entry_ok
even_2:                     ; 偶数FAT项处理
    and ax, 0x0fff           ; 需要将高四位清零（它是下一个FAT项的低四位）
get_fat_entry_ok:
    pop bx
    pop es
    ret
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
; 作用:关闭软驱马达，有时候软驱读取完如果不关闭马达，马达会持续运行且发出声音
; ----------------------------------------------------------------------------------------------------------------------
kill_motor:
 	push	dx
 	mov	dx, 03F2h
 	mov	al, 0
 	out	dx, al
 	pop	dx
 	ret
; ======================================================================================================================