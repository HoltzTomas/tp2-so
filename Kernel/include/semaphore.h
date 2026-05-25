#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "defs.h"

void sem_init_system(void);
int sem_open(const char *name, uint64_t initialValue);
int sem_wait(const char *name);
int sem_post(const char *name);
int sem_close(const char *name);

#endif
