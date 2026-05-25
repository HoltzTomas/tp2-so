#include "syscallDispatcher.h"
#include "process.h"
#include "scheduler.h"
#include "memoryManager.h"
#include "semaphore.h"
#include "pipe.h"
#include "keyboard.h"
#include "naiveConsole.h"
#include "lib.h"
#include "defs.h"

#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_MALLOC 2
#define SYS_FREE 3
#define SYS_CREATE_PROCESS 4
#define SYS_EXIT 5
#define SYS_GETPID 6
#define SYS_KILL 7
#define SYS_BLOCK 8
#define SYS_UNBLOCK 9
#define SYS_YIELD 10
#define SYS_SET_PRIORITY 11
#define SYS_LIST_PROCESSES 12
#define SYS_SEM_OPEN 13
#define SYS_SEM_WAIT 14
#define SYS_SEM_POST 15
#define SYS_SEM_CLOSE 16
#define SYS_PIPE_CREATE 17
#define SYS_PIPE_OPEN 18
#define SYS_PIPE_CLOSE 19
#define SYS_MEM_INFO 20
#define SYS_WAIT 21
#define SYS_CLEAR 22

void keyboard_handler_wrapper(uint64_t scancode) {
    keyboard_add_char((uint8_t)scancode);
}

static uint64_t sys_read(uint64_t fd, uint64_t buf, uint64_t count) {
    pid_t pid = process_getpid();
    fd_t realFd;

    if (fd == STDIN) {
        realFd = process_get_fd(pid, 0);
    } else {
        realFd = (fd_t)fd;
    }

    if (realFd == STDIN) {
        char *buffer = (char *)buf;
        for (uint64_t i = 0; i < count; i++) {
            char c = keyboard_get_char();
            if (c == (char)EOF) {
                return i;
            }
            buffer[i] = c;
        }
        return count;
    } else {
        return (uint64_t)pipe_read(realFd, (char *)buf, (int)count);
    }
}

static uint64_t sys_write(uint64_t fd, uint64_t buf, uint64_t count) {
    pid_t pid = process_getpid();
    fd_t realFd;

    if (fd == STDOUT) {
        realFd = process_get_fd(pid, 1);
    } else {
        realFd = (fd_t)fd;
    }

    if (realFd == STDOUT) {
        const char *buffer = (const char *)buf;
        for (uint64_t i = 0; i < count; i++) {
            ncPrintChar(buffer[i]);
        }
        return count;
    } else {
        return (uint64_t)pipe_write(realFd, (const char *)buf, (int)count);
    }
}

uint64_t syscall_dispatcher(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t rax) {
    switch (rax) {
    case SYS_READ:
        return sys_read(rdi, rsi, rdx);

    case SYS_WRITE:
        return sys_write(rdi, rsi, rdx);

    case SYS_MALLOC:
        return (uint64_t)mm_malloc(rdi);

    case SYS_FREE:
        mm_free((void *)rdi);
        return 0;

    case SYS_CREATE_PROCESS: {
        ProcessFunc func = (ProcessFunc)rdi;
        uint64_t argc = rsi;
        char **argv = (char **)rdx;
        const char *name = (const char *)rcx;
        uint8_t foreground = (uint8_t)r8;
        fd_t fds[2] = {process_get_fd(process_getpid(), 0), process_get_fd(process_getpid(), 1)};
        return (uint64_t)process_create(func, argc, argv, name, foreground, fds);
    }

    case SYS_EXIT:
        process_exit((int)rdi);
        return 0;

    case SYS_GETPID:
        return (uint64_t)process_getpid();

    case SYS_KILL:
        return (uint64_t)process_kill((pid_t)rdi);

    case SYS_BLOCK:
        return (uint64_t)process_block((pid_t)rdi);

    case SYS_UNBLOCK:
        return (uint64_t)process_unblock((pid_t)rdi);

    case SYS_YIELD:
        scheduler_yield();
        process_yield();
        return 0;

    case SYS_SET_PRIORITY:
        return (uint64_t)process_set_priority((pid_t)rdi, (uint8_t)rsi);

    case SYS_LIST_PROCESSES:
        return (uint64_t)process_list((ProcessInfo *)rdi, (int)rsi);

    case SYS_SEM_OPEN:
        return (uint64_t)sem_open((const char *)rdi, rsi);

    case SYS_SEM_WAIT:
        return (uint64_t)sem_wait((const char *)rdi);

    case SYS_SEM_POST:
        return (uint64_t)sem_post((const char *)rdi);

    case SYS_SEM_CLOSE:
        return (uint64_t)sem_close((const char *)rdi);

    case SYS_PIPE_CREATE:
        return (uint64_t)pipe_create((fd_t *)rdi);

    case SYS_PIPE_OPEN:
        return (uint64_t)pipe_open((const char *)rdi, (fd_t *)rsi);

    case SYS_PIPE_CLOSE:
        return (uint64_t)pipe_close((fd_t)rdi);

    case SYS_MEM_INFO:
        mm_get_info((MemoryInfo *)rdi);
        return 0;

    case SYS_WAIT:
        return (uint64_t)process_wait((pid_t)rdi);

    case SYS_CLEAR:
        ncClear();
        return 0;

    default:
        return (uint64_t)-1;
    }
}
