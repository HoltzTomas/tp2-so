#include "pipe.h"
#include "process.h"
#include "interrupts.h"
#include "lib.h"

typedef struct {
    char buffer[PIPE_BUFFER_SIZE];
    int readPos;
    int writePos;
    int count;
    uint8_t active;
    int readFd;
    int writeFd;
    int readOpen;
    int writeOpen;
    pid_t blockedReader;
    pid_t blockedWriter;
    uint8_t lock;
} Pipe;

static Pipe pipes[MAX_PIPES];
static int nextFd = PIPE_FD_BASE;

static void acquire_lock(uint8_t *lock) {
    while (__sync_lock_test_and_set(lock, 1))
        ;
}

static void release_lock(uint8_t *lock) {
    __sync_lock_release(lock);
}

void pipe_init_system(void) {
    memset(pipes, 0, sizeof(pipes));
    nextFd = PIPE_FD_BASE;
}

static int find_pipe_by_fd(int fd) {
    for (int i = 0; i < MAX_PIPES; i++) {
        if (pipes[i].active && (pipes[i].readFd == fd || pipes[i].writeFd == fd))
            return i;
    }
    return -1;
}

int pipe_is_pipe(int fd) {
    return fd >= PIPE_FD_BASE && find_pipe_by_fd(fd) >= 0;
}

int pipe_create(int fds[2]) {
    int slot = -1;
    for (int i = 0; i < MAX_PIPES; i++) {
        if (!pipes[i].active) {
            slot = i;
            break;
        }
    }
    if (slot < 0)
        return -1;

    Pipe *p = &pipes[slot];
    memset(p, 0, sizeof(Pipe));
    p->active = 1;
    p->readFd = nextFd++;
    p->writeFd = nextFd++;
    p->readOpen = 1;
    p->writeOpen = 1;
    p->blockedReader = -1;
    p->blockedWriter = -1;

    fds[0] = p->readFd;
    fds[1] = p->writeFd;
    return 0;
}

int pipe_write(int fd, const char *buf, int count) {
    int idx = find_pipe_by_fd(fd);
    if (idx < 0)
        return -1;

    Pipe *p = &pipes[idx];
    if (fd != p->writeFd)
        return -1;
    if (!p->readOpen)
        return -1; /* Broken pipe - no readers */

    int written = 0;
    while (written < count) {
        acquire_lock(&p->lock);

        if (p->count < PIPE_BUFFER_SIZE) {
            p->buffer[p->writePos] = buf[written];
            p->writePos = (p->writePos + 1) % PIPE_BUFFER_SIZE;
            p->count++;
            written++;

            /* Wake blocked reader if any */
            if (p->blockedReader >= 0) {
                pid_t reader = p->blockedReader;
                p->blockedReader = -1;
                release_lock(&p->lock);
                process_unblock(reader);
                continue;
            }
            release_lock(&p->lock);
        } else {
            /* Buffer full - block writer */
            p->blockedWriter = process_getpid();
            release_lock(&p->lock);
            process_block(process_getpid());
        }
    }
    return written;
}

int pipe_read(int fd, char *buf, int count) {
    int idx = find_pipe_by_fd(fd);
    if (idx < 0)
        return -1;

    Pipe *p = &pipes[idx];
    if (fd != p->readFd)
        return -1;

    int bytesRead = 0;
    while (bytesRead < count) {
        acquire_lock(&p->lock);

        if (p->count > 0) {
            buf[bytesRead] = p->buffer[p->readPos];
            p->readPos = (p->readPos + 1) % PIPE_BUFFER_SIZE;
            p->count--;
            bytesRead++;

            /* Wake blocked writer if any */
            if (p->blockedWriter >= 0) {
                pid_t writer = p->blockedWriter;
                p->blockedWriter = -1;
                release_lock(&p->lock);
                process_unblock(writer);
                continue;
            }
            release_lock(&p->lock);
        } else {
            /* Buffer empty */
            if (!p->writeOpen) {
                /* EOF - write end closed */
                release_lock(&p->lock);
                return bytesRead;
            }
            /* Block reader */
            p->blockedReader = process_getpid();
            release_lock(&p->lock);
            process_block(process_getpid());
        }
    }
    return bytesRead;
}

int pipe_close(int fd) {
    int idx = find_pipe_by_fd(fd);
    if (idx < 0)
        return -1;

    Pipe *p = &pipes[idx];
    acquire_lock(&p->lock);

    if (fd == p->readFd) {
        p->readOpen = 0;
        /* Wake blocked writer - write will get broken pipe */
        if (p->blockedWriter >= 0) {
            pid_t writer = p->blockedWriter;
            p->blockedWriter = -1;
            release_lock(&p->lock);
            process_unblock(writer);
            goto check_destroy;
        }
    } else if (fd == p->writeFd) {
        p->writeOpen = 0;
        /* Wake blocked reader - they'll get EOF */
        if (p->blockedReader >= 0) {
            pid_t reader = p->blockedReader;
            p->blockedReader = -1;
            release_lock(&p->lock);
            process_unblock(reader);
            goto check_destroy;
        }
    }
    release_lock(&p->lock);

check_destroy:
    if (!p->readOpen && !p->writeOpen) {
        p->active = 0;
    }
    return 0;
}
