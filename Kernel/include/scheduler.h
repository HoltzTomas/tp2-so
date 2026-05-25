#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "defs.h"

void scheduler_init(void);
pid_t scheduler_next(void);
void scheduler_add(pid_t pid);
void scheduler_remove(pid_t pid);
void scheduler_tick(void);
void scheduler_set_priority(pid_t pid, uint8_t priority);
uint8_t scheduler_get_priority(pid_t pid);

#endif
