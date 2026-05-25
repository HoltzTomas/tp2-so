GLOBAL sys_read
GLOBAL sys_write
GLOBAL sys_malloc
GLOBAL sys_free
GLOBAL sys_create_process
GLOBAL sys_exit
GLOBAL sys_getpid
GLOBAL sys_kill
GLOBAL sys_block
GLOBAL sys_unblock
GLOBAL sys_yield
GLOBAL sys_wait
GLOBAL sys_nice
GLOBAL sys_list_processes
GLOBAL sys_mem_info
GLOBAL sys_sleep
GLOBAL sys_get_ticks

section .text

; int64_t sys_read(int64_t fd, char *buf, uint64_t count)
sys_read:
    mov rax, 0
    int 80h
    ret

; int64_t sys_write(int64_t fd, const char *buf, uint64_t count)
sys_write:
    mov rax, 1
    int 80h
    ret

; void *sys_malloc(uint64_t size)
sys_malloc:
    mov rax, 2
    int 80h
    ret

; void sys_free(void *ptr)
sys_free:
    mov rax, 3
    int 80h
    ret

; int64_t sys_create_process(void *func, uint64_t argc, char *argv[], const char *name, uint64_t extra)
; rdi=func, rsi=argc, rdx=argv, rcx=name, r8=extra
sys_create_process:
    mov rax, 4
    int 80h
    ret

; void sys_exit(int64_t retValue)
sys_exit:
    mov rax, 5
    int 80h
    ret

; int64_t sys_getpid(void)
sys_getpid:
    mov rax, 6
    int 80h
    ret

; int64_t sys_kill(int64_t pid)
sys_kill:
    mov rax, 7
    int 80h
    ret

; int64_t sys_block(int64_t pid)
sys_block:
    mov rax, 8
    int 80h
    ret

; int64_t sys_unblock(int64_t pid)
sys_unblock:
    mov rax, 9
    int 80h
    ret

; void sys_yield(void)
sys_yield:
    mov rax, 10
    int 80h
    ret

; int64_t sys_wait(int64_t pid)
sys_wait:
    mov rax, 11
    int 80h
    ret

; int64_t sys_nice(int64_t pid, int64_t priority)
sys_nice:
    mov rax, 12
    int 80h
    ret

; int64_t sys_list_processes(void *buf, int64_t max)
sys_list_processes:
    mov rax, 20
    int 80h
    ret

; int64_t sys_mem_info(void *info)
sys_mem_info:
    mov rax, 21
    int 80h
    ret

; int64_t sys_sleep(uint64_t ms)
sys_sleep:
    mov rax, 22
    int 80h
    ret

; uint64_t sys_get_ticks(void)
sys_get_ticks:
    mov rax, 23
    int 80h
    ret
