#ifndef SYSCALL_DISPATCHER_H
#define SYSCALL_DISPATCHER_H

#include <stdint.h>

uint64_t syscall_dispatcher(uint64_t arg0, uint64_t arg1, uint64_t arg2,
                            uint64_t arg3, uint64_t arg4, uint64_t syscall_nr);

#endif
