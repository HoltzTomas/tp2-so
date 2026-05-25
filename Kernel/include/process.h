#ifndef PROCESS_H
#define PROCESS_H

#include "defs.h"

typedef struct {
    pid_t pid;
    char name[PROCESS_NAME_LEN];
    ProcessState state;
    uint8_t priority;
    uint8_t foreground;
    pid_t parentPid;
    void *stackBase;
    void *rsp;
    int16_t fds[2]; /* 0=stdin, 1=stdout */
    int retValue;
    uint16_t quantumsLeft;
    pid_t waitingFor; /* PID this process is waiting for via waitpid */
} PCB;

void process_init(void);
pid_t process_create(ProcessFunc func, uint64_t argc, char *argv[],
                     const char *name, uint8_t foreground, int16_t fds[2]);
int process_kill(pid_t pid);
int process_block(pid_t pid);
int process_unblock(pid_t pid);
void process_exit(int retValue);
int process_wait(pid_t pid);
pid_t process_getpid(void);
int process_nice(pid_t pid, uint8_t priority);
int process_list(ProcessInfo *buf, int max);
PCB *process_get_current(void);
void process_yield(void);
int process_set_fds(pid_t pid, int16_t fds[2]);

#endif
