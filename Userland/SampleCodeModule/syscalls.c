#include "include/syscalls.h"

int sys_read(int fd, char *buf, int count) {
    return (int)syscall_invoke(SYS_READ, (uint64_t)fd, (uint64_t)buf, (uint64_t)count, 0, 0);
}

int sys_write(int fd, const char *buf, int count) {
    return (int)syscall_invoke(SYS_WRITE, (uint64_t)fd, (uint64_t)buf, (uint64_t)count, 0, 0);
}

void *sys_malloc(uint64_t size) {
    return (void *)syscall_invoke(SYS_MALLOC, size, 0, 0, 0, 0);
}

void sys_free(void *ptr) {
    syscall_invoke(SYS_FREE, (uint64_t)ptr, 0, 0, 0, 0);
}

int64_t sys_create_process(uint64_t func, uint64_t argc, char *argv[], const char *name, uint8_t foreground) {
    return (int64_t)syscall_invoke(SYS_CREATE_PROCESS, func, argc, (uint64_t)argv, (uint64_t)name, (uint64_t)foreground);
}

void sys_exit(int retValue) {
    syscall_invoke(SYS_EXIT, (uint64_t)retValue, 0, 0, 0, 0);
}

int64_t sys_getpid(void) {
    return (int64_t)syscall_invoke(SYS_GETPID, 0, 0, 0, 0, 0);
}

int sys_kill(int64_t pid) {
    return (int)syscall_invoke(SYS_KILL, (uint64_t)pid, 0, 0, 0, 0);
}

int sys_block(int64_t pid) {
    return (int)syscall_invoke(SYS_BLOCK, (uint64_t)pid, 0, 0, 0, 0);
}

int sys_unblock(int64_t pid) {
    return (int)syscall_invoke(SYS_UNBLOCK, (uint64_t)pid, 0, 0, 0, 0);
}

void sys_yield(void) {
    syscall_invoke(SYS_YIELD, 0, 0, 0, 0, 0);
}

int sys_set_priority(int64_t pid, uint8_t priority) {
    return (int)syscall_invoke(SYS_SET_PRIORITY, (uint64_t)pid, (uint64_t)priority, 0, 0, 0);
}

int sys_list_processes(void *buffer, int maxCount) {
    return (int)syscall_invoke(SYS_LIST_PROCESSES, (uint64_t)buffer, (uint64_t)maxCount, 0, 0, 0);
}

int sys_sem_open(const char *name, uint64_t initialValue) {
    return (int)syscall_invoke(SYS_SEM_OPEN, (uint64_t)name, initialValue, 0, 0, 0);
}

int sys_sem_wait(const char *name) {
    return (int)syscall_invoke(SYS_SEM_WAIT, (uint64_t)name, 0, 0, 0, 0);
}

int sys_sem_post(const char *name) {
    return (int)syscall_invoke(SYS_SEM_POST, (uint64_t)name, 0, 0, 0, 0);
}

int sys_sem_close(const char *name) {
    return (int)syscall_invoke(SYS_SEM_CLOSE, (uint64_t)name, 0, 0, 0, 0);
}

int sys_pipe_create(int fds[2]) {
    return (int)syscall_invoke(SYS_PIPE_CREATE, (uint64_t)fds, 0, 0, 0, 0);
}

int sys_pipe_open(const char *name, int fds[2]) {
    return (int)syscall_invoke(SYS_PIPE_OPEN, (uint64_t)name, (uint64_t)fds, 0, 0, 0);
}

int sys_pipe_close(int fd) {
    return (int)syscall_invoke(SYS_PIPE_CLOSE, (uint64_t)fd, 0, 0, 0, 0);
}

void sys_mem_info(void *info) {
    syscall_invoke(SYS_MEM_INFO, (uint64_t)info, 0, 0, 0, 0);
}

int sys_wait(int64_t pid) {
    return (int)syscall_invoke(SYS_WAIT, (uint64_t)pid, 0, 0, 0, 0);
}

void sys_clear(void) {
    syscall_invoke(SYS_CLEAR, 0, 0, 0, 0, 0);
}
