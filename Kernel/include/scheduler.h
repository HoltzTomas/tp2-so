#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "defs.h"

void scheduler_init(void);
pid_t scheduler_next(void);
void scheduler_add(pid_t pid, uint8_t priority);
void scheduler_remove(pid_t pid);
void scheduler_block(pid_t pid);
void scheduler_unblock(pid_t pid);
void scheduler_set_priority(pid_t pid, uint8_t priority);
void scheduler_yield(void);
pid_t scheduler_current(void);
uint8_t scheduler_get_priority(pid_t pid);

#endif
