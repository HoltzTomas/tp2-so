#include "../include/libc.h"
#include "../include/syscalls.h"
#include "../include/test_util.h"

#define MAX_PROCESSES 10

enum State { RUNNING_STATE, BLOCKED_STATE, KILLED_STATE };

typedef struct {
    int64_t pid;
    enum State state;
} p_rq;

uint64_t test_proc(uint64_t argc, char *argv[]) {
    (void)argc;
    (void)argv;
    uint8_t rq;
    uint8_t alive = 0;
    uint8_t action;
    uint64_t max_processes = MAX_PROCESSES;

    p_rq p_rqs[MAX_PROCESSES];

    while (1) {
        /* Create max_processes running endless_loop */
        for (rq = 0; rq < max_processes; rq++) {
            char *args[] = {"endless_loop"};
            p_rqs[rq].pid = sys_create_process(
                (void *)endless_loop, 1, args, "endless_loop", 0);

            if (p_rqs[rq].pid < 0) {
                printf("test_proc: ERROR creating process\n");
                return (uint64_t)-1;
            }
            p_rqs[rq].state = RUNNING_STATE;
            alive++;
        }

        /* Randomly kill/block/unblock */
        while (alive > 0) {
            rq = GetUniform(max_processes - 1);

            action = GetUniform(2);

            switch (action) {
            case 0: /* Kill */
                if (p_rqs[rq].state != KILLED_STATE) {
                    if (sys_kill(p_rqs[rq].pid) < 0) {
                        printf("test_proc: ERROR killing PID %d\n",
                               (int)p_rqs[rq].pid);
                        return (uint64_t)-1;
                    }
                    p_rqs[rq].state = KILLED_STATE;
                    alive--;
                }
                break;

            case 1: /* Block */
                if (p_rqs[rq].state == RUNNING_STATE) {
                    if (sys_block(p_rqs[rq].pid) < 0) {
                        printf("test_proc: ERROR blocking PID %d\n",
                               (int)p_rqs[rq].pid);
                        return (uint64_t)-1;
                    }
                    p_rqs[rq].state = BLOCKED_STATE;
                }
                break;

            case 2: /* Unblock */
                if (p_rqs[rq].state == BLOCKED_STATE) {
                    if (sys_unblock(p_rqs[rq].pid) < 0) {
                        printf("test_proc: ERROR unblocking PID %d\n",
                               (int)p_rqs[rq].pid);
                        return (uint64_t)-1;
                    }
                    p_rqs[rq].state = RUNNING_STATE;
                }
                break;
            }
        }
        printf("test_proc OK\n");
        break;
    }
    return 0;
}
