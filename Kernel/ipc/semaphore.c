#include "semaphore.h"
#include "process.h"
#include "scheduler.h"
#include "interrupts.h"
#include "lib.h"

#define MAX_WAITING 64

typedef struct {
    char name[SEM_NAME_LEN];
    int64_t value;
    uint8_t active;
    uint16_t refCount;
    pid_t waitingQueue[MAX_WAITING];
    int waitCount;
    uint8_t lock;
} Semaphore;

static Semaphore semaphores[MAX_SEMAPHORES];

static void acquire_lock(uint8_t *lock) {
    while (__sync_lock_test_and_set(lock, 1)) {
        /* spin - very short, only protects kernel structure */
    }
}

static void release_lock(uint8_t *lock) {
    __sync_lock_release(lock);
}

void sem_init_system(void) {
    memset(semaphores, 0, sizeof(semaphores));
}

static int find_sem(const char *name) {
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (semaphores[i].active && strcmp(semaphores[i].name, name) == 0)
            return i;
    }
    return -1;
}

static int find_free_slot(void) {
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (!semaphores[i].active)
            return i;
    }
    return -1;
}

int sem_open(const char *name, uint64_t initialValue) {
    if (name == NULL)
        return -1;

    int idx = find_sem(name);
    if (idx >= 0) {
        acquire_lock(&semaphores[idx].lock);
        semaphores[idx].refCount++;
        release_lock(&semaphores[idx].lock);
        return 0;
    }

    idx = find_free_slot();
    if (idx < 0)
        return -1;

    Semaphore *sem = &semaphores[idx];
    acquire_lock(&sem->lock);
    strncpy(sem->name, name, SEM_NAME_LEN);
    sem->value = (int64_t)initialValue;
    sem->active = 1;
    sem->refCount = 1;
    sem->waitCount = 0;
    release_lock(&sem->lock);

    return 0;
}

int sem_wait(const char *name) {
    if (name == NULL)
        return -1;

    int idx = find_sem(name);
    if (idx < 0)
        return -1;

    Semaphore *sem = &semaphores[idx];
    acquire_lock(&sem->lock);

    sem->value--;

    if (sem->value < 0) {
        /* Block the current process */
        pid_t pid = process_getpid();
        if (sem->waitCount < MAX_WAITING) {
            sem->waitingQueue[sem->waitCount++] = pid;
        }
        release_lock(&sem->lock);

        /* Block process - this triggers a context switch */
        process_block(pid);
        return 0;
    }

    release_lock(&sem->lock);
    return 0;
}

int sem_post(const char *name) {
    if (name == NULL)
        return -1;

    int idx = find_sem(name);
    if (idx < 0)
        return -1;

    Semaphore *sem = &semaphores[idx];
    acquire_lock(&sem->lock);

    sem->value++;

    if (sem->value <= 0 && sem->waitCount > 0) {
        /* Wake up first waiting process (FIFO) */
        pid_t pid = sem->waitingQueue[0];

        /* Shift queue left */
        for (int i = 0; i < sem->waitCount - 1; i++) {
            sem->waitingQueue[i] = sem->waitingQueue[i + 1];
        }
        sem->waitCount--;

        release_lock(&sem->lock);
        process_unblock(pid);
        return 0;
    }

    release_lock(&sem->lock);
    return 0;
}

int sem_close(const char *name) {
    if (name == NULL)
        return -1;

    int idx = find_sem(name);
    if (idx < 0)
        return -1;

    Semaphore *sem = &semaphores[idx];
    acquire_lock(&sem->lock);

    sem->refCount--;
    if (sem->refCount == 0) {
        /* Wake all waiting processes before destroying */
        for (int i = 0; i < sem->waitCount; i++) {
            process_unblock(sem->waitingQueue[i]);
        }
        sem->active = 0;
        memset(sem->name, 0, SEM_NAME_LEN);
    }

    release_lock(&sem->lock);
    return 0;
}
