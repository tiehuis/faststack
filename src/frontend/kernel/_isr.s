global isr0

%macro ISR_ERR 1
global isr%1
isr%1:
    cli
    push byte 0
    push byte %1
    jmp isr_handler_setup
%endmacro

%macro ISR_NO_ERR 1
global isr%1
isr%1:
    cli
    push byte %1
    jmp isr_handler_setup
%endmacro

ISR_ERR 0
ISR_ERR 1
ISR_ERR 2
ISR_ERR 3
ISR_ERR 4
ISR_ERR 5
ISR_ERR 6
ISR_ERR 7
ISR_NO_ERR 8
ISR_ERR 9
ISR_NO_ERR 10
ISR_NO_ERR 11
ISR_NO_ERR 12
ISR_NO_ERR 13
ISR_NO_ERR 14
ISR_ERR 15
ISR_ERR 16
ISR_ERR 17
ISR_ERR 18
ISR_ERR 19
ISR_ERR 20
ISR_ERR 21
ISR_ERR 22
ISR_ERR 23
ISR_ERR 24
ISR_ERR 25
ISR_ERR 26
ISR_ERR 27
ISR_ERR 28
ISR_ERR 29
ISR_ERR 30
ISR_ERR 31

isr_handler_setup:
    pusha
    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    extern isr_handler
    call isr_handler

    pop eax,
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8
    sti
    iret
