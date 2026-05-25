#include "../include/test_util.h"
#include "../include/libc.h"
#include "../include/syscalls.h"

#define MAX_PROCS_TEST 64

enum State { P_RUNNING,
             P_BLOCKED,
             P_KILLED };

typedef struct P_rq {
    int32_t pid;
    enum State state;
} p_rq;

uint64_t test_processes(uint64_t argc, char *argv[]) {
    uint8_t rq;
    uint8_t alive = 0;
    uint8_t action;
    uint64_t max_processes;
    char *argvAux[] = {0};

    if (argc != 1)
        return -1;

    if ((max_processes = satoi(argv[0])) <= 0)
        return -1;

    p_rq p_rqs[MAX_PROCS_TEST];

    while (1) {
        for (rq = 0; rq < max_processes; rq++) {
            p_rqs[rq].pid = (int32_t)sys_create_process((uint64_t)endless_loop, 0, argvAux, "endless_loop", 0);

            if (p_rqs[rq].pid == -1) {
                printf("test_processes: ERROR creating process\n");
                return -1;
            } else {
                p_rqs[rq].state = P_RUNNING;
                alive++;
            }
        }

        while (alive > 0) {
            for (rq = 0; rq < max_processes; rq++) {
                action = GetUniform(100) % 2;

                switch (action) {
                case 0:
                    if (p_rqs[rq].state == P_RUNNING || p_rqs[rq].state == P_BLOCKED) {
                        if (sys_kill(p_rqs[rq].pid) == -1) {
                            printf("test_processes: ERROR killing process\n");
                            return -1;
                        }
                        p_rqs[rq].state = P_KILLED;
                        sys_wait(p_rqs[rq].pid);
                        alive--;
                    }
                    break;

                case 1:
                    if (p_rqs[rq].state == P_RUNNING) {
                        if (sys_block(p_rqs[rq].pid) == -1) {
                            printf("test_processes: ERROR blocking process\n");
                            return -1;
                        }
                        p_rqs[rq].state = P_BLOCKED;
                    }
                    break;
                }
            }

            for (rq = 0; rq < max_processes; rq++)
                if (p_rqs[rq].state == P_BLOCKED && GetUniform(100) % 2) {
                    if (sys_unblock(p_rqs[rq].pid) == -1) {
                        printf("test_processes: ERROR unblocking process\n");
                        return -1;
                    }
                    p_rqs[rq].state = P_RUNNING;
                }
        }
    }
}
