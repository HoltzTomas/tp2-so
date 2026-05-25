#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>

int64_t sys_read(int64_t fd, char *buf, uint64_t count);
int64_t sys_write(int64_t fd, const char *buf, uint64_t count);
void *sys_malloc(uint64_t size);
void sys_free(void *ptr);
int64_t sys_mem_info(void *info);
uint64_t sys_get_ticks(void);

#endif
