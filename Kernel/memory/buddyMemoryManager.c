#include "memoryManager.h"
#include "lib.h"

#define MAX_ORDER 20
#define MIN_BLOCK_SIZE 64

typedef struct BuddyBlock {
    struct BuddyBlock *next;
} BuddyBlock;

static BuddyBlock *freeLists[MAX_ORDER + 1];
static void *memoryStart;
static uint64_t totalSize;
static uint64_t usedMemory;
static int maxOrder;

static int get_order(uint64_t size) {
    int order = 0;
    uint64_t blockSize = MIN_BLOCK_SIZE;
    while (blockSize < size) {
        blockSize <<= 1;
        order++;
    }
    return order;
}

static uint64_t order_to_size(int order) {
    return (uint64_t)MIN_BLOCK_SIZE << order;
}

static void *buddy_of(void *block, int order) {
    uint64_t offset = (uint64_t)block - (uint64_t)memoryStart;
    uint64_t buddyOffset = offset ^ order_to_size(order);
    return (void *)((uint64_t)memoryStart + buddyOffset);
}

static void list_remove(BuddyBlock **list, BuddyBlock *block) {
    BuddyBlock *prev = NULL;
    BuddyBlock *current = *list;
    while (current != NULL) {
        if (current == block) {
            if (prev == NULL)
                *list = current->next;
            else
                prev->next = current->next;
            return;
        }
        prev = current;
        current = current->next;
    }
}

void mm_init(void *start, uint64_t size) {
    memoryStart = start;
    totalSize = size;
    usedMemory = 0;

    for (int i = 0; i <= MAX_ORDER; i++)
        freeLists[i] = NULL;

    maxOrder = 0;
    uint64_t blockSize = MIN_BLOCK_SIZE;
    while (blockSize < size && maxOrder < MAX_ORDER) {
        blockSize <<= 1;
        maxOrder++;
    }
    if (blockSize > size)
        maxOrder--;

    BuddyBlock *block = (BuddyBlock *)start;
    block->next = NULL;
    freeLists[maxOrder] = block;
}

void *mm_malloc(uint64_t size) {
    if (size == 0)
        return NULL;

    size += sizeof(uint64_t);

    int order = get_order(size);
    if (order > maxOrder)
        return NULL;

    int i;
    for (i = order; i <= maxOrder; i++) {
        if (freeLists[i] != NULL)
            break;
    }

    if (i > maxOrder)
        return NULL;

    BuddyBlock *block = freeLists[i];
    freeLists[i] = block->next;

    while (i > order) {
        i--;
        BuddyBlock *buddy = (BuddyBlock *)((uint8_t *)block + order_to_size(i));
        buddy->next = freeLists[i];
        freeLists[i] = buddy;
    }

    usedMemory += order_to_size(order);

    uint64_t *header = (uint64_t *)block;
    *header = (uint64_t)order;
    return (void *)(header + 1);
}

void mm_free(void *ptr) {
    if (ptr == NULL)
        return;

    uint64_t *header = (uint64_t *)ptr - 1;
    int order = (int)(*header);
    BuddyBlock *block = (BuddyBlock *)header;

    usedMemory -= order_to_size(order);

    while (order < maxOrder) {
        BuddyBlock *buddy = (BuddyBlock *)buddy_of(block, order);

        BuddyBlock *current = freeLists[order];
        int found = 0;
        while (current != NULL) {
            if (current == buddy) {
                found = 1;
                break;
            }
            current = current->next;
        }

        if (!found)
            break;

        list_remove(&freeLists[order], buddy);

        if ((uint64_t)buddy < (uint64_t)block)
            block = (BuddyBlock *)buddy;

        order++;
    }

    block->next = freeLists[order];
    freeLists[order] = block;
}

void mm_get_info(MemoryInfo *info) {
    info->totalMemory = totalSize;
    info->usedMemory = usedMemory;
    info->freeMemory = totalSize - usedMemory;
}
