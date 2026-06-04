#include "../include/libc.h"
#include "../include/syscalls.h"
#include "../include/test_util.h"

#define SEM_ID "test_sync_sem"
#define TOTAL_PAIR_PROCESSES 2
#define N 100000

static int64_t global;

static void slowInc(int64_t *p, int64_t inc) {
    int64_t aux = *p;
    sys_yield();
    aux += inc;
    *p = aux;
}

static void inc_process(uint64_t argc, char *argv[]) {
    (void)argc;
    (void)argv;
    uint64_t n = N;
    uint64_t i;

    for (i = 0; i < n; i++) {
        sys_sem_wait(SEM_ID);
        slowInc(&global, 1);
        sys_sem_post(SEM_ID);
    }
    sys_exit(0);
}

static void dec_process(uint64_t argc, char *argv[]) {
    (void)argc;
    (void)argv;
    uint64_t n = N;
    uint64_t i;

    for (i = 0; i < n; i++) {
        sys_sem_wait(SEM_ID);
        slowInc(&global, -1);
        sys_sem_post(SEM_ID);
    }
    sys_exit(0);
}

uint64_t test_sync(uint64_t argc, char *argv[]) {
    (void)argc;
    (void)argv;
    int64_t pids[TOTAL_PAIR_PROCESSES * 2];
    uint64_t i;

    global = 0;

    if (sys_sem_open(SEM_ID, 1) < 0) {
        printf("test_sync: ERROR opening semaphore\n");
        return (uint64_t)-1;
    }

    for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
        pids[i] = sys_create_process((void *)inc_process, 0, NULL,
                                     "inc_process", 0);
        pids[i + TOTAL_PAIR_PROCESSES] = sys_create_process(
            (void *)dec_process, 0, NULL, "dec_process", 0);
    }

    for (i = 0; i < TOTAL_PAIR_PROCESSES * 2; i++) {
        sys_wait(pids[i]);
    }

    sys_sem_close(SEM_ID);

    printf("Final value: %d\n", (int)global);
    if (global == 0) {
        printf("test_sync OK\n");
    } else {
        printf("test_sync ERROR (expected 0)\n");
    }
    return (uint64_t)(global != 0);
}
