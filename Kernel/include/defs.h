#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>

#define NULL ((void *)0)

#define MAX_PROCESSES 64
#define MAX_SEMAPHORES 64
#define MAX_PIPES 32
#define PIPE_BUFFER_SIZE 1024
#define PROCESS_STACK_SIZE 4096
#define PROCESS_NAME_LEN 32
#define SEM_NAME_LEN 32

#define MAX_PRIORITY 4
#define DEFAULT_PRIORITY 1

#define STDIN 0
#define STDOUT 1
#define PIPE_FD_BASE 100

typedef int16_t pid_t;
typedef int16_t fd_t;

typedef uint64_t (*ProcessFunc)(uint64_t argc, char *argv[]);

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    ZOMBIE
} ProcessState;

typedef struct {
    uint64_t totalMemory;
    uint64_t usedMemory;
    uint64_t freeMemory;
} MemoryInfo;

typedef struct {
    pid_t pid;
    char name[PROCESS_NAME_LEN];
    uint8_t priority;
    ProcessState state;
    uint8_t foreground;
    int16_t parentPid;
} ProcessInfo;

#endif
