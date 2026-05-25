#include "pipe.h"
#include "process.h"
#include "memoryManager.h"
#include "lib.h"
#include "interrupts.h"

typedef struct Pipe {
    char buffer[PIPE_BUFFER_SIZE];
    int readIdx;
    int writeIdx;
    int count;
    uint8_t active;
    pid_t readBlockedPid;
    pid_t writeBlockedPid;
    int readOpen;
    int writeOpen;
    char name[SEM_NAME_LEN];
} Pipe;

static Pipe pipes[MAX_PIPES];

#define FD_TO_PIPE_READ(fd) ((fd - 10) / 2)
#define FD_TO_PIPE_WRITE(fd) ((fd - 10) / 2)
#define IS_READ_FD(fd) ((fd >= 10) && ((fd - 10) % 2 == 0))
#define IS_WRITE_FD(fd) ((fd >= 10) && ((fd - 10) % 2 == 1))

void pipe_init_module(void) {
    for (int i = 0; i < MAX_PIPES; i++) {
        pipes[i].active = 0;
        pipes[i].readIdx = 0;
        pipes[i].writeIdx = 0;
        pipes[i].count = 0;
        pipes[i].readBlockedPid = -1;
        pipes[i].writeBlockedPid = -1;
        pipes[i].readOpen = 0;
        pipes[i].writeOpen = 0;
        pipes[i].name[0] = '\0';
    }
}

static int find_free_pipe(void) {
    for (int i = 0; i < MAX_PIPES; i++) {
        if (!pipes[i].active)
            return i;
    }
    return -1;
}

static int find_pipe_by_name(const char *name) {
    for (int i = 0; i < MAX_PIPES; i++) {
        if (pipes[i].active && strcmp(pipes[i].name, name) == 0)
            return i;
    }
    return -1;
}

int pipe_create(fd_t fds[2]) {
    int idx = find_free_pipe();
    if (idx == -1)
        return -1;

    pipes[idx].active = 1;
    pipes[idx].readIdx = 0;
    pipes[idx].writeIdx = 0;
    pipes[idx].count = 0;
    pipes[idx].readBlockedPid = -1;
    pipes[idx].writeBlockedPid = -1;
    pipes[idx].readOpen = 1;
    pipes[idx].writeOpen = 1;
    pipes[idx].name[0] = '\0';

    fds[0] = 10 + idx * 2;
    fds[1] = 10 + idx * 2 + 1;

    return 0;
}

int pipe_open(const char *name, fd_t fds[2]) {
    int idx = find_pipe_by_name(name);
    if (idx != -1) {
        fds[0] = 10 + idx * 2;
        fds[1] = 10 + idx * 2 + 1;
        return 0;
    }

    idx = find_free_pipe();
    if (idx == -1)
        return -1;

    pipes[idx].active = 1;
    pipes[idx].readIdx = 0;
    pipes[idx].writeIdx = 0;
    pipes[idx].count = 0;
    pipes[idx].readBlockedPid = -1;
    pipes[idx].writeBlockedPid = -1;
    pipes[idx].readOpen = 1;
    pipes[idx].writeOpen = 1;
    strncpy(pipes[idx].name, name, SEM_NAME_LEN - 1);
    pipes[idx].name[SEM_NAME_LEN - 1] = '\0';

    fds[0] = 10 + idx * 2;
    fds[1] = 10 + idx * 2 + 1;

    return 0;
}

int pipe_write(fd_t fd, const char *buf, int count) {
    if (!IS_WRITE_FD(fd) && fd != STDOUT)
        return -1;

    if (fd == STDOUT)
        return -1;

    int idx = FD_TO_PIPE_WRITE(fd);
    if (idx < 0 || idx >= MAX_PIPES || !pipes[idx].active)
        return -1;

    Pipe *p = &pipes[idx];
    int written = 0;

    for (int i = 0; i < count; i++) {
        while (p->count >= PIPE_BUFFER_SIZE) {
            p->writeBlockedPid = process_getpid();
            process_block(process_getpid());
        }

        p->buffer[p->writeIdx] = buf[i];
        p->writeIdx = (p->writeIdx + 1) % PIPE_BUFFER_SIZE;
        p->count++;
        written++;

        if (p->readBlockedPid != -1) {
            process_unblock(p->readBlockedPid);
            p->readBlockedPid = -1;
        }
    }

    return written;
}

int pipe_read(fd_t fd, char *buf, int count) {
    if (!IS_READ_FD(fd) && fd != STDIN)
        return -1;

    if (fd == STDIN)
        return -1;

    int idx = FD_TO_PIPE_READ(fd);
    if (idx < 0 || idx >= MAX_PIPES || !pipes[idx].active)
        return -1;

    Pipe *p = &pipes[idx];
    int bytesRead = 0;

    for (int i = 0; i < count; i++) {
        while (p->count == 0) {
            if (!p->writeOpen)
                return bytesRead;
            p->readBlockedPid = process_getpid();
            process_block(process_getpid());
        }

        buf[i] = p->buffer[p->readIdx];
        p->readIdx = (p->readIdx + 1) % PIPE_BUFFER_SIZE;
        p->count--;
        bytesRead++;

        if (p->writeBlockedPid != -1) {
            process_unblock(p->writeBlockedPid);
            p->writeBlockedPid = -1;
        }
    }

    return bytesRead;
}

int pipe_close(fd_t fd) {
    int idx;
    if (IS_READ_FD(fd)) {
        idx = FD_TO_PIPE_READ(fd);
        if (idx < 0 || idx >= MAX_PIPES || !pipes[idx].active)
            return -1;
        pipes[idx].readOpen = 0;
        if (pipes[idx].writeBlockedPid != -1) {
            process_unblock(pipes[idx].writeBlockedPid);
            pipes[idx].writeBlockedPid = -1;
        }
    } else if (IS_WRITE_FD(fd)) {
        idx = FD_TO_PIPE_WRITE(fd);
        if (idx < 0 || idx >= MAX_PIPES || !pipes[idx].active)
            return -1;
        pipes[idx].writeOpen = 0;
        if (pipes[idx].readBlockedPid != -1) {
            process_unblock(pipes[idx].readBlockedPid);
            pipes[idx].readBlockedPid = -1;
        }
    } else {
        return -1;
    }

    if (!pipes[idx].readOpen && !pipes[idx].writeOpen) {
        pipes[idx].active = 0;
    }

    return 0;
}
