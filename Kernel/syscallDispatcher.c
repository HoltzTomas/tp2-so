#include "syscallDispatcher.h"
#include "keyboard.h"
#include "video.h"
#include "memoryManager.h"
#include "process.h"
#include "interrupts.h"
#include "defs.h"
#include "lib.h"

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
#define SYS_WAIT 11
#define SYS_NICE 12
#define SYS_LIST_PROCESSES 20
#define SYS_MEM_INFO 21
#define SYS_SLEEP 22
#define SYS_GET_TICKS 23

static uint64_t tick_counter = 0;

uint64_t get_ticks(void) {
    return tick_counter;
}

void increment_ticks(void) {
    tick_counter++;
}

uint64_t syscall_dispatcher(uint64_t arg0, uint64_t arg1, uint64_t arg2,
                            uint64_t arg3, uint64_t arg4, uint64_t syscall_nr) {
    (void)arg4;

    switch (syscall_nr) {
    case SYS_READ: {
        if ((int64_t)arg0 == STDIN) {
            char *buf = (char *)arg1;
            uint64_t count = arg2;
            uint64_t i;
            for (i = 0; i < count; i++) {
                buf[i] = keyboard_get_char();
            }
            return i;
        }
        return 0;
    }
    case SYS_WRITE: {
        if ((int64_t)arg0 == STDOUT) {
            const char *buf = (const char *)arg1;
            uint64_t count = arg2;
            for (uint64_t i = 0; i < count; i++) {
                video_put_char(buf[i]);
            }
            return count;
        }
        return 0;
    }
    case SYS_MALLOC: {
        void *ptr = mm_malloc(arg0);
        return (uint64_t)ptr;
    }
    case SYS_FREE: {
        mm_free((void *)arg0);
        return 0;
    }
    case SYS_CREATE_PROCESS: {
        /* arg0=func, arg1=argc, arg2=argv, arg3=name, arg4=foreground|fds packed */
        /* Simplified: arg0=func, arg1=argc, arg2=argv, arg3=name */
        ProcessFunc func = (ProcessFunc)arg0;
        uint64_t argc = arg1;
        char **argv = (char **)arg2;
        const char *name = (const char *)arg3;
        uint8_t fg = (uint8_t)(arg4 & 0xFF);
        int16_t *fds = NULL;
        if (arg4 >> 8) {
            fds = (int16_t *)(arg4 >> 8);
        }
        return (uint64_t)process_create(func, argc, argv, name, fg, fds);
    }
    case SYS_EXIT: {
        process_exit((int)arg0);
        return 0;
    }
    case SYS_GETPID: {
        return (uint64_t)process_getpid();
    }
    case SYS_KILL: {
        return (uint64_t)process_kill((pid_t)arg0);
    }
    case SYS_BLOCK: {
        return (uint64_t)process_block((pid_t)arg0);
    }
    case SYS_UNBLOCK: {
        return (uint64_t)process_unblock((pid_t)arg0);
    }
    case SYS_YIELD: {
        process_yield();
        return 0;
    }
    case SYS_WAIT: {
        return (uint64_t)process_wait((pid_t)arg0);
    }
    case SYS_NICE: {
        return (uint64_t)process_nice((pid_t)arg0, (uint8_t)arg1);
    }
    case SYS_LIST_PROCESSES: {
        return (uint64_t)process_list((ProcessInfo *)arg0, (int)arg1);
    }
    case SYS_MEM_INFO: {
        mm_get_info((MemoryInfo *)arg0);
        return 0;
    }
    case SYS_SLEEP: {
        /* Simple busy-wait sleep - will improve later */
        uint64_t target = tick_counter + arg0 / 55; /* ~55ms per tick at 18.2Hz */
        while (tick_counter < target) {
            _hlt();
        }
        return 0;
    }
    case SYS_GET_TICKS: {
        return get_ticks();
    }
    default:
        return (uint64_t)-1;
    }
}
