#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdint.h>
#include "defs.h"

void mm_init(void *start, uint64_t size);
void *mm_malloc(uint64_t size);
void mm_free(void *ptr);
void mm_get_info(MemoryInfo *info);

#endif
