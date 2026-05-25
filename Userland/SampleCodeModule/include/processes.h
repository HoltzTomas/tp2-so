#ifndef PROCESSES_H
#define PROCESSES_H

#include <stdint.h>

#define MAX_PROCESSES 64
#define PROCESS_NAME_LEN 32

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    ZOMBIE
} ProcessState;

typedef struct ProcessInfo {
    int pid;
    char name[PROCESS_NAME_LEN];
    uint8_t priority;
    uint64_t rsp;
    uint64_t rbp;
    uint8_t foreground;
    ProcessState state;
    int parentPid;
} ProcessInfo;

typedef struct MemoryInfo {
    uint64_t totalMemory;
    uint64_t usedMemory;
    uint64_t freeMemory;
} MemoryInfo;

#endif
