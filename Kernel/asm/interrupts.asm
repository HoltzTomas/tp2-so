GLOBAL _cli
GLOBAL _sti
GLOBAL _hlt
GLOBAL pic_master_eoi
GLOBAL irq_init
GLOBAL irq_enable
GLOBAL irq_disable
GLOBAL force_timer
GLOBAL _initialize_stack_frame

GLOBAL _irq00_handler
GLOBAL _irq01_handler
GLOBAL _syscall_handler

EXTERN schedule
EXTERN keyboard_handler
EXTERN syscall_dispatcher

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
    sti
    hlt
    ret

pic_master_eoi:
    push rax
    mov al, 0x20
    out 0x20, al
    pop rax
    ret

; Timer interrupt handler (IRQ0) - performs context switch
_irq00_handler:
    push_state

    mov rdi, rsp          ; Pass current RSP to schedule()
    call schedule         ; Returns new RSP in RAX
    mov rsp, rax          ; Switch to new process stack

    ; Send EOI
    mov al, 0x20
    out 0x20, al

    pop_state
    iretq

; Keyboard interrupt handler (IRQ1)
_irq01_handler:
    push_state

    in al, 0x60
    movzx rdi, al
    call keyboard_handler

    ; Send EOI
    mov al, 0x20
    out 0x20, al

    pop_state
    iretq

; Syscall handler (INT 0x80)
_syscall_handler:
    push_state

    ; Args: rdi=arg0, rsi=arg1, rdx=arg2, rcx=arg3, r8=arg4, rax=syscall_nr
    mov r9, rax       ; syscall_nr as 6th arg

    call syscall_dispatcher

    ; Store return value - overwrite rax on stack (rax is at top after push_state = offset 14*8)
    mov [rsp + 14*8], rax

    pop_state
    iretq

; Force a timer interrupt (triggers context switch)
force_timer:
    int 0x20
    ret

; Initialize a stack frame for a new process
; void *_initialize_stack_frame(void (*func)(uint64_t, char**), void *args, void *stack_top)
; rdi = function pointer (entry point)
; rsi = args (will be passed as argc in rdi when process starts)
; rdx = stack top
;
; Sets up the stack so that when pop_state + iretq executes, it jumps to func
; with rdi=args (argc), rsi=0 (argv placeholder)
_initialize_stack_frame:
    mov rax, rdx          ; rax = stack_top

    ; iretq frame (pushed in reverse order on stack, so SS is at highest addr)
    sub rax, 8
    mov qword [rax], 0x0          ; SS
    sub rax, 8
    mov [rax], rdx                ; RSP (original stack top)
    sub rax, 8
    mov qword [rax], 0x202        ; RFLAGS (IF=1)
    sub rax, 8
    mov qword [rax], 0x08         ; CS (kernel code segment)
    sub rax, 8
    mov [rax], rdi                ; RIP = function entry point

    ; push_state frame (15 registers): rax,rbx,rcx,rdx,rbp,rdi,rsi,r8-r15
    sub rax, 8
    mov qword [rax], 0            ; RAX
    sub rax, 8
    mov qword [rax], 0            ; RBX
    sub rax, 8
    mov qword [rax], 0            ; RCX
    sub rax, 8
    mov qword [rax], 0            ; RDX
    sub rax, 8
    mov qword [rax], 0            ; RBP
    sub rax, 8
    mov [rax], rsi                ; RDI = args (argc)
    sub rax, 8
    mov qword [rax], 0            ; RSI = 0 (argv)
    sub rax, 8
    mov qword [rax], 0            ; R8
    sub rax, 8
    mov qword [rax], 0            ; R9
    sub rax, 8
    mov qword [rax], 0            ; R10
    sub rax, 8
    mov qword [rax], 0            ; R11
    sub rax, 8
    mov qword [rax], 0            ; R12
    sub rax, 8
    mov qword [rax], 0            ; R13
    sub rax, 8
    mov qword [rax], 0            ; R14
    sub rax, 8
    mov qword [rax], 0            ; R15

    ; RAX now points to the "top" of the initialized stack
    ret

; PIC initialization
irq_init:
    ; ICW1
    mov al, 0x11
    out 0x20, al
    out 0xA0, al

    ; ICW2 - set interrupt vector offset
    mov al, 0x20
    out 0x21, al
    mov al, 0x28
    out 0xA1, al

    ; ICW3
    mov al, 0x04
    out 0x21, al
    mov al, 0x02
    out 0xA1, al

    ; ICW4
    mov al, 0x01
    out 0x21, al
    out 0xA1, al

    ; Set up IDT entries
    call setup_idt

    ; Enable IRQ0 (timer) and IRQ1 (keyboard)
    mov al, 0xFC
    out 0x21, al
    mov al, 0xFF
    out 0xA1, al

    ret

irq_enable:
    sti
    ret

irq_disable:
    cli
    ret

; IDT
SECTION .data
align 16
idt:
    times 256 dq 0, 0

idt_descriptor:
    dw 256 * 16 - 1
    dq idt

SECTION .text

%macro idt_entry 2
    mov rax, %2
    mov word [idt + %1*16], ax
    mov word [idt + %1*16 + 2], 0x08
    mov byte [idt + %1*16 + 4], 0
    mov byte [idt + %1*16 + 5], 0x8E
    shr rax, 16
    mov word [idt + %1*16 + 6], ax
    shr rax, 16
    mov dword [idt + %1*16 + 8], eax
    mov dword [idt + %1*16 + 12], 0
%endmacro

%macro idt_entry_user 2
    mov rax, %2
    mov word [idt + %1*16], ax
    mov word [idt + %1*16 + 2], 0x08
    mov byte [idt + %1*16 + 4], 0
    mov byte [idt + %1*16 + 5], 0xEE
    shr rax, 16
    mov word [idt + %1*16 + 6], ax
    shr rax, 16
    mov dword [idt + %1*16 + 8], eax
    mov dword [idt + %1*16 + 12], 0
%endmacro

setup_idt:
    idt_entry 0x20, _irq00_handler
    idt_entry 0x21, _irq01_handler
    idt_entry_user 0x80, _syscall_handler

    lidt [idt_descriptor]
    ret
