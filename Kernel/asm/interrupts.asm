GLOBAL _cli
GLOBAL _sti
GLOBAL _hlt
GLOBAL pic_master_eoi
GLOBAL pic_slave_eoi
GLOBAL irq_init
GLOBAL irq_enable
GLOBAL irq_disable
GLOBAL force_timer

GLOBAL _irq00_handler
GLOBAL _irq01_handler
GLOBAL _syscall_handler

EXTERN timer_handler_wrapper
EXTERN keyboard_handler_wrapper
EXTERN syscall_dispatcher
EXTERN schedule

SECTION .text

%macro push_state 0
    push rax
    push rbx
    push rcx
    push rdx
    push rbp
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro pop_state 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rbp
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

_cli:
    cli
    ret

_sti:
    sti
    ret

_hlt:
    hlt
    ret

pic_master_eoi:
    push rax
    mov al, 0x20
    out 0x20, al
    pop rax
    ret

pic_slave_eoi:
    push rax
    mov al, 0x20
    out 0xA0, al
    out 0x20, al
    pop rax
    ret

; Timer interrupt handler (IRQ0)
_irq00_handler:
    push_state

    mov rdi, rsp        ; pass current stack pointer
    call schedule       ; get new stack pointer
    mov rsp, rax        ; switch to new stack

    mov al, 0x20
    out 0x20, al        ; send EOI

    pop_state
    iretq

; Keyboard interrupt handler (IRQ1)
_irq01_handler:
    push_state

    in al, 0x60        ; read scancode from keyboard
    movzx rdi, al
    call keyboard_handler_wrapper

    mov al, 0x20
    out 0x20, al        ; send EOI

    pop_state
    iretq

; Syscall handler (INT 0x80)
_syscall_handler:
    push_state

    mov r9, rax         ; syscall number as 6th arg
    push r9
    mov r9, r8          ; 5th param
    mov r8, rcx         ; 4th param (r10 -> rcx was clobbered)
    mov rcx, rdx        ; 3rd param
    mov rdx, rsi        ; 2nd param
                        ; rdi already has 1st param
    pop r9              ; restore syscall number
    ; rdi=arg0, rsi=arg1, rdx=arg2, rcx=arg3, r8=arg4, r9=syscall_nr
    push rbp
    mov rbp, rsp
    call syscall_dispatcher
    pop rbp

    ; Store return value - replace rax on stack
    mov [rsp + 15*8], rax   ; rax is first pushed (last on pop)

    pop_state
    iretq

; Force a timer interrupt to start scheduling
force_timer:
    int 0x20
    ret

; PIC initialization
irq_init:
    ; ICW1
    mov al, 0x11
    out 0x20, al        ; Master PIC
    out 0xA0, al        ; Slave PIC

    ; ICW2 - set interrupt vector offset
    mov al, 0x20        ; Master offset: 0x20 (IRQ0 -> INT 32)
    out 0x21, al
    mov al, 0x28        ; Slave offset: 0x28 (IRQ8 -> INT 40)
    out 0xA1, al

    ; ICW3
    mov al, 0x04        ; Master: slave on IRQ2
    out 0x21, al
    mov al, 0x02        ; Slave: cascade identity
    out 0xA1, al

    ; ICW4
    mov al, 0x01        ; 8086 mode
    out 0x21, al
    out 0xA1, al

    ; Set up IDT entries for timer, keyboard, and syscall
    call setup_idt

    ; Mask all interrupts except timer (IRQ0) and keyboard (IRQ1)
    mov al, 0xFC        ; Enable IRQ0 and IRQ1 only
    out 0x21, al
    mov al, 0xFF        ; Disable all on slave
    out 0xA1, al

    ret

irq_enable:
    sti
    ret

irq_disable:
    cli
    ret

; IDT setup
SECTION .data
align 16
idt:
    times 256 dq 0, 0   ; 256 entries, 16 bytes each

idt_descriptor:
    dw 256 * 16 - 1     ; limit
    dq idt              ; base address

SECTION .text

%macro idt_entry 2
    ; %1 = vector number, %2 = handler address
    mov rax, %2
    mov word [idt + %1*16], ax        ; offset low
    mov word [idt + %1*16 + 2], 0x08  ; selector (kernel code segment)
    mov byte [idt + %1*16 + 4], 0     ; IST = 0
    mov byte [idt + %1*16 + 5], 0x8E  ; type: interrupt gate, DPL=0, present
    shr rax, 16
    mov word [idt + %1*16 + 6], ax    ; offset mid
    shr rax, 16
    mov dword [idt + %1*16 + 8], eax  ; offset high
    mov dword [idt + %1*16 + 12], 0   ; reserved
%endmacro

%macro idt_entry_user 2
    ; Same but DPL=3 (accessible from ring 3)
    mov rax, %2
    mov word [idt + %1*16], ax
    mov word [idt + %1*16 + 2], 0x08
    mov byte [idt + %1*16 + 4], 0
    mov byte [idt + %1*16 + 5], 0xEE  ; type: interrupt gate, DPL=3, present
    shr rax, 16
    mov word [idt + %1*16 + 6], ax
    shr rax, 16
    mov dword [idt + %1*16 + 8], eax
    mov dword [idt + %1*16 + 12], 0
%endmacro

setup_idt:
    idt_entry 0x20, _irq00_handler      ; Timer
    idt_entry 0x21, _irq01_handler      ; Keyboard
    idt_entry_user 0x80, _syscall_handler  ; Syscall (user accessible)

    lidt [idt_descriptor]
    ret
