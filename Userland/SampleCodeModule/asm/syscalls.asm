GLOBAL sys_read
GLOBAL sys_write
GLOBAL sys_malloc
GLOBAL sys_free
GLOBAL sys_mem_info
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

; int64_t sys_mem_info(void *info)
sys_mem_info:
    mov rax, 21
    int 80h
    ret

; uint64_t sys_get_ticks(void)
sys_get_ticks:
    mov rax, 23
    int 80h
    ret
