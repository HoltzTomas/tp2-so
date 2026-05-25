#include "../include/test_util.h"
#include "../include/libc.h"
#include "../include/syscalls.h"

#define SEM_ID "test_sync_sem"
#define TOTAL_PAIR_PROCESSES 2

static int64_t global_sync;

static void slowInc(int64_t *p, int64_t inc) {
    uint64_t aux = *p;
    if (GetUniform(100) < 30)
        sys_yield();
    aux += inc;
    *p = aux;
}

static uint64_t my_process_inc(uint64_t argc, char *argv[]) {
    uint64_t n;
    int8_t inc;
    int8_t use_sem;

    if (argc != 3)
        return -1;

    if ((n = satoi(argv[0])) <= 0)
        return -1;
    if ((inc = satoi(argv[1])) == 0)
        return -1;
    if ((use_sem = satoi(argv[2])) < 0)
        return -1;

    if (use_sem)
        if (!sys_sem_open(SEM_ID, 1)) {
            printf("test_sync: ERROR opening semaphore\n");
            return -1;
        }

    uint64_t i;
    for (i = 0; i < n; i++) {
        if (use_sem)
            sys_sem_wait(SEM_ID);
        slowInc(&global_sync, inc);
        if (use_sem)
            sys_sem_post(SEM_ID);
    }

    if (use_sem)
        sys_sem_close(SEM_ID);

    return 0;
}

uint64_t test_sync(uint64_t argc, char *argv[]) {
    uint64_t pids[2 * TOTAL_PAIR_PROCESSES];

    if (argc != 2)
        return -1;

    char *argvDec[] = {argv[0], "-1", argv[1], 0};
    char *argvInc[] = {argv[0], "1", argv[1], 0};

    global_sync = 0;

    uint64_t i;
    for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
        pids[i] = sys_create_process((uint64_t)my_process_inc, 3, argvDec, "process_inc", 0);
        pids[i + TOTAL_PAIR_PROCESSES] = sys_create_process((uint64_t)my_process_inc, 3, argvInc, "process_inc", 0);
    }

    for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
        sys_wait(pids[i]);
        sys_wait(pids[i + TOTAL_PAIR_PROCESSES]);
    }

    printf("Final value: %d\n", global_sync);

    return 0;
}
