#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>

#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_MALLOC 2
#define SYS_FREE 3
#define SYS_CREATE_PROCESS 4
#define SYS_EXIT 5
#define SYS_GETPID 6
#define SYS_KILL 7
#define SYS_BLOCK 8
#define SYS_UNBLOCK 9
#define SYS_YIELD 10
#define SYS_SET_PRIORITY 11
#define SYS_LIST_PROCESSES 12
#define SYS_SEM_OPEN 13
#define SYS_SEM_WAIT 14
#define SYS_SEM_POST 15
#define SYS_SEM_CLOSE 16
#define SYS_PIPE_CREATE 17
#define SYS_PIPE_OPEN 18
#define SYS_PIPE_CLOSE 19
#define SYS_MEM_INFO 20
#define SYS_WAIT 21
#define SYS_CLEAR 22

extern uint64_t syscall_invoke(uint64_t rax, uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8);

int sys_read(int fd, char *buf, int count);
int sys_write(int fd, const char *buf, int count);
void *sys_malloc(uint64_t size);
void sys_free(void *ptr);
int64_t sys_create_process(uint64_t func, uint64_t argc, char *argv[], const char *name, uint8_t foreground);
void sys_exit(int retValue);
int64_t sys_getpid(void);
int sys_kill(int64_t pid);
int sys_block(int64_t pid);
int sys_unblock(int64_t pid);
void sys_yield(void);
int sys_set_priority(int64_t pid, uint8_t priority);
int sys_list_processes(void *buffer, int maxCount);
int sys_sem_open(const char *name, uint64_t initialValue);
int sys_sem_wait(const char *name);
int sys_sem_post(const char *name);
int sys_sem_close(const char *name);
int sys_pipe_create(int fds[2]);
int sys_pipe_open(const char *name, int fds[2]);
int sys_pipe_close(int fd);
void sys_mem_info(void *info);
int sys_wait(int64_t pid);
void sys_clear(void);

#endif
