#include "syscallDispatcher.h"
#include "keyboard.h"
#include "video.h"
#include "memoryManager.h"
#include "defs.h"

#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_MALLOC 2
#define SYS_FREE 3
#define SYS_MEM_INFO 21
#define SYS_GET_TICKS 23

static uint64_t tick_counter = 0;

void timer_tick(void) {
    tick_counter++;
}

uint64_t get_ticks(void) {
    return tick_counter;
}

uint64_t syscall_dispatcher(uint64_t arg0, uint64_t arg1, uint64_t arg2,
                            uint64_t arg3, uint64_t arg4, uint64_t syscall_nr) {
    (void)arg3;
    (void)arg4;

    switch (syscall_nr) {
    case SYS_READ: {
        /* arg0=fd, arg1=buf, arg2=count */
        if (arg0 == STDIN) {
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
        /* arg0=fd, arg1=buf, arg2=count */
        if (arg0 == STDOUT) {
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
    case SYS_MEM_INFO: {
        mm_get_info((MemoryInfo *)arg0);
        return 0;
    }
    case SYS_GET_TICKS: {
        return get_ticks();
    }
    default:
        return (uint64_t)-1;
    }
}
