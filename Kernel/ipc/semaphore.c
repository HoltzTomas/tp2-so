#include "semaphore.h"
#include "process.h"
#include "memoryManager.h"
#include "lib.h"
#include "interrupts.h"

typedef struct WaitQueue {
    pid_t pid;
    struct WaitQueue *next;
} WaitQueue;

typedef struct Semaphore {
    char name[SEM_NAME_LEN];
    int64_t value;
    uint8_t active;
    uint16_t openCount;
    WaitQueue *waitQueue;
    uint8_t lock;
} Semaphore;

static Semaphore semaphores[MAX_SEMAPHORES];

static void acquire_lock(uint8_t *lock) {
    while (__sync_lock_test_and_set(lock, 1))
        ;
}

static void release_lock(uint8_t *lock) {
    __sync_lock_release(lock);
}

void sem_init_module(void) {
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        semaphores[i].active = 0;
        semaphores[i].name[0] = '\0';
        semaphores[i].value = 0;
        semaphores[i].openCount = 0;
        semaphores[i].waitQueue = NULL;
        semaphores[i].lock = 0;
    }
}

static int find_sem(const char *name) {
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (semaphores[i].active && strcmp(semaphores[i].name, name) == 0)
            return i;
    }
    return -1;
}

static int find_free_sem(void) {
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (!semaphores[i].active)
            return i;
    }
    return -1;
}

int sem_open(const char *name, uint64_t initialValue) {
    _cli();
    int idx = find_sem(name);
    if (idx != -1) {
        semaphores[idx].openCount++;
        _sti();
        return 1;
    }

    idx = find_free_sem();
    if (idx == -1) {
        _sti();
        return 0;
    }

    strncpy(semaphores[idx].name, name, SEM_NAME_LEN - 1);
    semaphores[idx].name[SEM_NAME_LEN - 1] = '\0';
    semaphores[idx].value = (int64_t)initialValue;
    semaphores[idx].active = 1;
    semaphores[idx].openCount = 1;
    semaphores[idx].waitQueue = NULL;
    semaphores[idx].lock = 0;
    _sti();
    return 1;
}

int sem_wait(const char *name) {
    int idx = find_sem(name);
    if (idx == -1)
        return -1;

    Semaphore *sem = &semaphores[idx];
    acquire_lock(&sem->lock);

    sem->value--;
    if (sem->value < 0) {
        pid_t pid = process_getpid();
        WaitQueue *node = (WaitQueue *)mm_malloc(sizeof(WaitQueue));
        if (node == NULL) {
            release_lock(&sem->lock);
            return -1;
        }
        node->pid = pid;
        node->next = NULL;

        if (sem->waitQueue == NULL) {
            sem->waitQueue = node;
        } else {
            WaitQueue *last = sem->waitQueue;
            while (last->next != NULL)
                last = last->next;
            last->next = node;
        }

        release_lock(&sem->lock);
        process_block(pid);
        return 0;
    }

    release_lock(&sem->lock);
    return 0;
}

int sem_post(const char *name) {
    int idx = find_sem(name);
    if (idx == -1)
        return -1;

    Semaphore *sem = &semaphores[idx];
    acquire_lock(&sem->lock);

    sem->value++;
    if (sem->value <= 0 && sem->waitQueue != NULL) {
        WaitQueue *node = sem->waitQueue;
        sem->waitQueue = node->next;
        pid_t pid = node->pid;
        mm_free(node);
        release_lock(&sem->lock);
        process_unblock(pid);
        return 0;
    }

    release_lock(&sem->lock);
    return 0;
}

int sem_close(const char *name) {
    int idx = find_sem(name);
    if (idx == -1)
        return -1;

    Semaphore *sem = &semaphores[idx];
    acquire_lock(&sem->lock);

    sem->openCount--;
    if (sem->openCount == 0) {
        while (sem->waitQueue != NULL) {
            WaitQueue *node = sem->waitQueue;
            sem->waitQueue = node->next;
            process_unblock(node->pid);
            mm_free(node);
        }
        sem->active = 0;
        sem->name[0] = '\0';
    }

    release_lock(&sem->lock);
    return 0;
}
