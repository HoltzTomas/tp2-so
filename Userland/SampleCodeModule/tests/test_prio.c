#include "../include/test_util.h"
#include "../include/libc.h"
#include "../include/syscalls.h"

#define TOTAL_PROCESSES 3
#define LOWEST 0
#define MEDIUM 1
#define HIGHEST 4

static int64_t prio[TOTAL_PROCESSES] = {LOWEST, MEDIUM, HIGHEST};
static uint64_t max_value_prio = 0;

static uint64_t zero_to_max(uint64_t argc, char *argv[]) {
    uint64_t value = 0;
    while (value++ != max_value_prio)
        ;
    printf("PROCESS %d DONE!\n", sys_getpid());
    return 0;
}

uint64_t test_prio(uint64_t argc, char *argv[]) {
    int64_t pids[TOTAL_PROCESSES];
    char *ztm_argv[] = {0};
    uint64_t i;

    if (argc != 1)
        return -1;

    if ((max_value_prio = satoi(argv[0])) <= 0)
        return -1;

    printf("SAME PRIORITY...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++)
        pids[i] = sys_create_process((uint64_t)zero_to_max, 0, ztm_argv, "zero_to_max", 0);

    for (i = 0; i < TOTAL_PROCESSES; i++)
        sys_wait(pids[i]);

    printf("\nSAME PRIORITY, THEN CHANGE IT...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++) {
        pids[i] = sys_create_process((uint64_t)zero_to_max, 0, ztm_argv, "zero_to_max", 0);
        sys_set_priority(pids[i], prio[i]);
        printf("  PROCESS %d NEW PRIORITY: %d\n", pids[i], prio[i]);
    }

    for (i = 0; i < TOTAL_PROCESSES; i++)
        sys_wait(pids[i]);

    printf("\nSAME PRIORITY, THEN CHANGE IT WHILE BLOCKED...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++) {
        pids[i] = sys_create_process((uint64_t)zero_to_max, 0, ztm_argv, "zero_to_max", 0);
        sys_block(pids[i]);
        sys_set_priority(pids[i], prio[i]);
        printf("  PROCESS %d NEW PRIORITY: %d\n", pids[i], prio[i]);
    }

    for (i = 0; i < TOTAL_PROCESSES; i++)
        sys_unblock(pids[i]);

    for (i = 0; i < TOTAL_PROCESSES; i++)
        sys_wait(pids[i]);

    return 0;
}
