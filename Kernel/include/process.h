#ifndef PROCESS_H
#define PROCESS_H

#include "defs.h"

typedef struct ProcessInfo {
    pid_t pid;
    char name[PROCESS_NAME_LEN];
    uint8_t priority;
    uint64_t rsp;
    uint64_t rbp;
    uint8_t foreground;
    ProcessState state;
    pid_t parentPid;
} ProcessInfo;

void process_init(void);
pid_t process_create(ProcessFunc func, uint64_t argc, char *argv[], const char *name, uint8_t foreground, fd_t fds[2]);
int process_kill(pid_t pid);
int process_block(pid_t pid);
int process_unblock(pid_t pid);
int process_set_priority(pid_t pid, uint8_t priority);
pid_t process_getpid(void);
int process_list(ProcessInfo *info_array, int max_count);
int process_wait(pid_t pid);
void process_yield(void);
void process_exit(int retValue);
ProcessState process_get_state(pid_t pid);

fd_t process_get_fd(pid_t pid, int index);
void process_set_fd(pid_t pid, int index, fd_t fd);
pid_t process_get_foreground(void);

#endif
