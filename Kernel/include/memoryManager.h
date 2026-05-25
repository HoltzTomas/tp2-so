#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdint.h>
#include <stddef.h>

typedef struct MemoryInfo {
    uint64_t totalMemory;
    uint64_t usedMemory;
    uint64_t freeMemory;
} MemoryInfo;

void mm_init(void *start, uint64_t size);
void *mm_malloc(uint64_t size);
void mm_free(void *ptr);
void mm_get_info(MemoryInfo *info);

#endif
