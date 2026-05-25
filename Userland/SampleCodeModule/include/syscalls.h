#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>

int64_t sys_read(int64_t fd, char *buf, uint64_t count);
int64_t sys_write(int64_t fd, const char *buf, uint64_t count);
void *sys_malloc(uint64_t size);
void sys_free(void *ptr);
int64_t sys_create_process(void *func, uint64_t argc, char *argv[],
                           const char *name, uint64_t extra);
void sys_exit(int64_t retValue);
int64_t sys_getpid(void);
int64_t sys_kill(int64_t pid);
int64_t sys_block(int64_t pid);
int64_t sys_unblock(int64_t pid);
void sys_yield(void);
int64_t sys_wait(int64_t pid);
int64_t sys_nice(int64_t pid, int64_t priority);
int64_t sys_sem_open(const char *name, uint64_t initialValue);
int64_t sys_sem_wait(const char *name);
int64_t sys_sem_post(const char *name);
int64_t sys_sem_close(const char *name);
int64_t sys_pipe_create(int fds[2]);
int64_t sys_pipe_close(int64_t fd);
int64_t sys_list_processes(void *buf, int64_t max);
int64_t sys_mem_info(void *info);
int64_t sys_sleep(uint64_t ms);
uint64_t sys_get_ticks(void);

#endif
