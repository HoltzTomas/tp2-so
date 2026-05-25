#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>
#include <stddef.h>

#define MAX_PROCESSES 64
#define MAX_PRIORITY 4
#define DEFAULT_PRIORITY 1
#define STACK_SIZE 4096
#define MAX_SEMAPHORES 64
#define MAX_PIPES 64
#define PIPE_BUFFER_SIZE 1024
#define SEM_NAME_LEN 32
#define MAX_FD 2
#define PROCESS_NAME_LEN 32
#define MAX_OPEN_SEMS_PER_PROC 16

#define STDIN 0
#define STDOUT 1

#define EOF -1

typedef int pid_t;
typedef int fd_t;

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    ZOMBIE
} ProcessState;

typedef uint64_t (*ProcessFunc)(uint64_t argc, char *argv[]);

#endif
