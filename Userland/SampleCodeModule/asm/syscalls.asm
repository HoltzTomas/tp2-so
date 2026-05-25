GLOBAL syscall_invoke

section .text

; uint64_t syscall_invoke(uint64_t rax, uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8)
; Parameters: rdi=rax_val, rsi=rdi_val, rdx=rsi_val, rcx=rdx_val, r8=rcx_val, r9=r8_val
syscall_invoke:
    push rbp
    mov rbp, rsp

    mov rax, rdi        ; syscall number
    mov r9, [rbp + 16]  ; 6th param (r8 value for syscall)
    mov rdi, rsi        ; 1st syscall param
    mov rsi, rdx        ; 2nd syscall param
    mov rdx, rcx        ; 3rd syscall param
    mov rcx, r8         ; 4th syscall param
    mov r8, r9          ; 5th syscall param

    int 0x80

    mov rsp, rbp
    pop rbp
    ret
