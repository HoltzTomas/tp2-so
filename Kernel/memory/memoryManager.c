#include "memoryManager.h"
#include "lib.h"

typedef struct MemBlock {
    uint64_t size;
    struct MemBlock *next;
    uint8_t free;
} MemBlock;

#define BLOCK_HEADER_SIZE sizeof(MemBlock)
#define ALIGN8(x) (((x) + 7) & ~7)

static MemBlock *freeList = NULL;
static uint64_t totalMemory = 0;
static uint64_t usedMemory = 0;

void mm_init(void *start, uint64_t size) {
    totalMemory = size;
    usedMemory = 0;
    freeList = (MemBlock *)start;
    freeList->size = size - BLOCK_HEADER_SIZE;
    freeList->next = NULL;
    freeList->free = 1;
}

void *mm_malloc(uint64_t size) {
    if (size == 0)
        return NULL;

    size = ALIGN8(size);

    MemBlock *current = freeList;

    while (current != NULL) {
        if (current->free && current->size >= size) {
            if (current->size >= size + BLOCK_HEADER_SIZE + 8) {
                MemBlock *newBlock = (MemBlock *)((uint8_t *)current + BLOCK_HEADER_SIZE + size);
                newBlock->size = current->size - size - BLOCK_HEADER_SIZE;
                newBlock->next = current->next;
                newBlock->free = 1;
                current->size = size;
                current->next = newBlock;
            }
            current->free = 0;
            usedMemory += current->size + BLOCK_HEADER_SIZE;
            return (void *)((uint8_t *)current + BLOCK_HEADER_SIZE);
        }
        current = current->next;
    }
    return NULL;
}

static void coalesce(void) {
    MemBlock *current = freeList;
    while (current != NULL && current->next != NULL) {
        if (current->free && current->next->free) {
            current->size += BLOCK_HEADER_SIZE + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void mm_free(void *ptr) {
    if (ptr == NULL)
        return;

    MemBlock *block = (MemBlock *)((uint8_t *)ptr - BLOCK_HEADER_SIZE);
    if (block->free)
        return;

    block->free = 1;
    usedMemory -= block->size + BLOCK_HEADER_SIZE;
    coalesce();
}

void mm_get_info(MemoryInfo *info) {
    info->totalMemory = totalMemory;
    info->usedMemory = usedMemory;
    info->freeMemory = totalMemory - usedMemory;
}
