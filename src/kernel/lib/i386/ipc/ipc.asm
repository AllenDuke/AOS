[section .lib]

; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   导出库函数
; ----------------------------------------------------------------------------------------------------------------------
global  send
global  receive
global  send_rec
global  in_outbox

global  park
global  unpark

; ----------------------------------------------------------------------------------------------------------------------
;   导入
; ----------------------------------------------------------------------------------------------------------------------
;extern g_unparkPid
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   常量定义，请查看 core/constant.h
; ----------------------------------------------------------------------------------------------------------------------
SEND            equ     0x1
RECEIVE         equ     0x2
SEND_REC        equ     0x3
IN_OUTBOX       equ     0x4
SYS_VEC         equ     0x94                ; 系统调用向量

SRC_DEST_MSGP   equ     4 + 4 + 4           ; ebx + ecx + eip
MSG_PTR         equ     SRC_DEST_MSGP + 4

PARK_VEC        equ     0x31
UNPARK_VEC      equ     0x32
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   执行系统调用 SEND
; 本例程执行系统调用函数，op为SEND，系统调用原型:int sys_call(op, src_dest)
; 本例程只是对op = SEND的封装。
; ----------------------------------------------------------------------------------------------------------------------
send:
    ; ebx/ecx 会被用到，所以我们将其保存一下
    push ebx
    push ecx

    mov ecx, SEND                   ; ecx = 调用操作是发送消息，op = SEND
    jmp com                         ; 公共的处理
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   系统调用 RECIIVE
; 本例程执行系统调用函数，op 为 RECIIVE
; ----------------------------------------------------------------------------------------------------------------------
receive:
    ; ebx/ecx 会被用到，所以我们将其保存一下
    push ebx
    push ecx

    mov ecx, RECEIVE                ; ecx = 调用操作是接收消息，op = RECEIVE
    jmp com                         ; 公共的处理
; ======================================================================================================================


; ======================================================================================================================
;   执行系统调用 SEND_REC
; 本例程执行系统调用函数，op 为 SEND_REC
; ----------------------------------------------------------------------------------------------------------------------
send_rec:
    ; ebx/ecx 会被用到，所以我们将其保存一下
    push ebx
    push ecx

    mov ecx, SEND_REC               ; ecx = 调用操作是发送消息并等待对方响应，op = SEND_REC
    jmp com
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   执行系统调用 IN_OUTBOX
; 本例程执行系统调用函数，op 为 IN_OUTBOX
; ----------------------------------------------------------------------------------------------------------------------
in_outbox:
    ; ebx/ecx 会被用到，所以我们将其保存一下
    push ebx
    push ecx

    mov ecx, IN_OUTBOX              ; ecx = 调用操作是设置发件箱，op = OUTBOX
; ----------------------------------------------------------------------------------------------------------------------
; 公共处理
; ----------------------------------------------------------------------------------------------------------------------
com:
    mov eax, [esp + SRC_DEST_MSGP]          ; eax = src_dest_msgp
    mov ebx, [esp + MSG_PTR]                ; ebx = msg_ptr
    int SYS_VEC                             ; 执行系统调用

    pop ecx
    pop ebx
    ret
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   park
; unready the caller process
; ----------------------------------------------------------------------------------------------------------------------
park:
    int PARK_VEC
    ret
; ======================================================================================================================


; ======================================================================================================================
; ----------------------------------------------------------------------------------------------------------------------
;   unpark
; ready
; ----------------------------------------------------------------------------------------------------------------------
unpark:
    mov eax, [esp+4]
;    mov [g_unparkPid], eax
    int UNPARK_VEC
    ret
; ======================================================================================================================