#include "../include/libc.h"
#include "../include/syscalls.h"
#include "../include/test_util.h"

#define MINOR_WAIT 1000000
#define WAIT 10000000

#define TOTAL_PROCESSES 3
#define LOWEST 0
#define MEDIUM 2
#define HIGHEST 4

static int64_t pids[TOTAL_PROCESSES];

static void endless_loop_print(uint64_t argc, char *argv[]) {
    (void)argc;
    int64_t pid = sys_getpid();
    while (1) {
        printf("%d ", (int)pid);
        bussy_wait(MINOR_WAIT);
        sys_yield();
    }
}

uint64_t test_prio(uint64_t argc, char *argv[]) {
    (void)argc;
    (void)argv;
    uint64_t i;

    printf("Creating %d processes with same priority\n", TOTAL_PROCESSES);
    for (i = 0; i < TOTAL_PROCESSES; i++) {
        pids[i] = sys_create_process((void *)endless_loop_print, 0, NULL,
                                     "prio_test", 0);
    }

    bussy_wait(WAIT);
    printf("\nChanging priorities...\n");

    for (i = 0; i < TOTAL_PROCESSES; i++) {
        switch (i) {
        case 0:
            sys_nice(pids[i], LOWEST);
            break;
        case 1:
            sys_nice(pids[i], MEDIUM);
            break;
        case 2:
            sys_nice(pids[i], HIGHEST);
            break;
        }
    }

    bussy_wait(WAIT);
    printf("\ntest_prio: Check output - higher priority PIDs should appear more\n");

    /* Kill all */
    for (i = 0; i < TOTAL_PROCESSES; i++) {
        sys_kill(pids[i]);
    }

    printf("test_prio DONE\n");
    return 0;
}
