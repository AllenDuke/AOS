org 0x100

    mov ax, 0b800h
    mov gs, ax
    mov ah, 0fh
    mov al, 'L'
    mov [gs:((80 * 0 + 39) * 2)], ax
    jmp $
