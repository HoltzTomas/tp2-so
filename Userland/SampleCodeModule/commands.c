#include "include/commands.h"
#include "include/libc.h"
#include "include/syscalls.h"
#include "include/processes.h"
#include "include/test_util.h"

uint64_t cmd_help(uint64_t argc, char *argv[]) {
    printf("Available commands:\n");
    printf("  help          - Show this help message\n");
    printf("  mem           - Display memory status\n");
    printf("  ps            - List all processes\n");
    printf("  loop          - Print PID periodically\n");
    printf("  kill <pid>    - Kill a process\n");
    printf("  nice <pid> <prio> - Change process priority\n");
    printf("  block <pid>   - Toggle block/unblock process\n");
    printf("  cat           - Print stdin as received\n");
    printf("  wc            - Count lines from input\n");
    printf("  filter        - Filter vowels from input\n");
    printf("  mvar <w> <r>  - Readers/writers problem\n");
    printf("  clear         - Clear screen\n");
    printf("\nTests:\n");
    printf("  test_mm <maxmem>     - Memory manager test\n");
    printf("  test_proc <maxproc>  - Process creation test\n");
    printf("  test_prio <maxval>   - Priority test\n");
    printf("  test_sync <n> <sem>  - Synchronization test\n");
    printf("\nUse & at end for background, | for pipe\n");
    return 0;
}

uint64_t cmd_mem(uint64_t argc, char *argv[]) {
    MemoryInfo info;
    sys_mem_info(&info);
    printf("Memory Status:\n");
    printf("  Total:  %u bytes\n", (uint64_t)info.totalMemory);
    printf("  Used:   %u bytes\n", (uint64_t)info.usedMemory);
    printf("  Free:   %u bytes\n", (uint64_t)info.freeMemory);
    return 0;
}

uint64_t cmd_ps(uint64_t argc, char *argv[]) {
    ProcessInfo procs[MAX_PROCESSES];
    int count = sys_list_processes(procs, MAX_PROCESSES);

    printf("PID  Name             Prio  State    FG   RSP              RBP\n");
    printf("---  ----             ----  -----    --   ---              ---\n");

    for (int i = 0; i < count; i++) {
        const char *state;
        switch (procs[i].state) {
        case READY:
            state = "READY";
            break;
        case RUNNING:
            state = "RUNNING";
            break;
        case BLOCKED:
            state = "BLOCKED";
            break;
        default:
            state = "ZOMBIE";
            break;
        }
        printf("%d    %s", (int64_t)procs[i].pid, procs[i].name);
        int nameLen = strlen(procs[i].name);
        for (int j = nameLen; j < 17; j++)
            putchar(' ');
        printf("%d     %s", (int64_t)procs[i].priority, state);
        int stateLen = strlen(state);
        for (int j = stateLen; j < 9; j++)
            putchar(' ');
        printf("%d    0x%x  0x%x\n", (int64_t)procs[i].foreground, procs[i].rsp, procs[i].rbp);
    }
    return 0;
}

uint64_t cmd_loop(uint64_t argc, char *argv[]) {
    int64_t pid = sys_getpid();
    while (1) {
        printf("Hello from process %d\n", pid);
        bussy_wait(50000000);
    }
    return 0;
}

uint64_t cmd_kill(uint64_t argc, char *argv[]) {
    if (argc < 1) {
        printf("Usage: kill <pid>\n");
        return 1;
    }
    int64_t pid = atoi(argv[0]);
    if (sys_kill(pid) == -1) {
        printf("Error: could not kill process %d\n", pid);
        return 1;
    }
    printf("Process %d killed\n", pid);
    return 0;
}

uint64_t cmd_nice(uint64_t argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: nice <pid> <priority>\n");
        return 1;
    }
    int64_t pid = atoi(argv[0]);
    uint8_t prio = (uint8_t)atoi(argv[1]);
    if (sys_set_priority(pid, prio) == -1) {
        printf("Error: could not change priority\n");
        return 1;
    }
    printf("Process %d priority set to %d\n", pid, (int64_t)prio);
    return 0;
}

uint64_t cmd_block(uint64_t argc, char *argv[]) {
    if (argc < 1) {
        printf("Usage: block <pid>\n");
        return 1;
    }
    int64_t pid = atoi(argv[0]);
    ProcessInfo procs[MAX_PROCESSES];
    int count = sys_list_processes(procs, MAX_PROCESSES);

    for (int i = 0; i < count; i++) {
        if (procs[i].pid == (int)pid) {
            if (procs[i].state == BLOCKED) {
                sys_unblock(pid);
                printf("Process %d unblocked\n", pid);
            } else {
                sys_block(pid);
                printf("Process %d blocked\n", pid);
            }
            return 0;
        }
    }
    printf("Process %d not found\n", pid);
    return 1;
}

uint64_t cmd_cat(uint64_t argc, char *argv[]) {
    char c;
    while (1) {
        int n = sys_read(STDIN, &c, 1);
        if (n <= 0)
            break;
        sys_write(STDOUT, &c, 1);
    }
    return 0;
}

uint64_t cmd_wc(uint64_t argc, char *argv[]) {
    char c;
    int lines = 0;
    while (1) {
        int n = sys_read(STDIN, &c, 1);
        if (n <= 0)
            break;
        if (c == '\n')
            lines++;
    }
    printf("Lines: %d\n", (int64_t)lines);
    return 0;
}

uint64_t cmd_filter(uint64_t argc, char *argv[]) {
    char c;
    while (1) {
        int n = sys_read(STDIN, &c, 1);
        if (n <= 0)
            break;
        if (c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u' &&
            c != 'A' && c != 'E' && c != 'I' && c != 'O' && c != 'U') {
            sys_write(STDOUT, &c, 1);
        }
    }
    return 0;
}

#define MVAR_SEM_WRITE "mvar_write"
#define MVAR_SEM_READ "mvar_read"
#define MVAR_SEM_MUTEX "mvar_mutex"

static int64_t mvar_value = 0;
static int mvar_readers = 0;

static uint64_t mvar_writer(uint64_t argc, char *argv[]) {
    int iterations = 100;
    for (int i = 0; i < iterations; i++) {
        bussy_wait(GetUniform(5000000));
        sys_sem_wait(MVAR_SEM_WRITE);
        mvar_value++;
        printf("Writer %d wrote: %d\n", sys_getpid(), mvar_value);
        sys_sem_post(MVAR_SEM_WRITE);
    }
    return 0;
}

static uint64_t mvar_reader(uint64_t argc, char *argv[]) {
    int iterations = 100;
    for (int i = 0; i < iterations; i++) {
        bussy_wait(GetUniform(5000000));
        sys_sem_wait(MVAR_SEM_MUTEX);
        mvar_readers++;
        if (mvar_readers == 1)
            sys_sem_wait(MVAR_SEM_WRITE);
        sys_sem_post(MVAR_SEM_MUTEX);

        printf("Reader %d read: %d\n", sys_getpid(), mvar_value);

        sys_sem_wait(MVAR_SEM_MUTEX);
        mvar_readers--;
        if (mvar_readers == 0)
            sys_sem_post(MVAR_SEM_WRITE);
        sys_sem_post(MVAR_SEM_MUTEX);
    }
    return 0;
}

uint64_t cmd_mvar(uint64_t argc, char *argv[]) {
    int writers = 2, readers = 3;
    if (argc >= 1)
        writers = atoi(argv[0]);
    if (argc >= 2)
        readers = atoi(argv[1]);

    mvar_value = 0;
    mvar_readers = 0;

    sys_sem_open(MVAR_SEM_WRITE, 1);
    sys_sem_open(MVAR_SEM_MUTEX, 1);

    int64_t pids[20];
    int total = 0;

    for (int i = 0; i < writers && total < 20; i++) {
        pids[total++] = sys_create_process((uint64_t)mvar_writer, 0, (char *[]){0}, "mvar_writer", 0);
    }
    for (int i = 0; i < readers && total < 20; i++) {
        pids[total++] = sys_create_process((uint64_t)mvar_reader, 0, (char *[]){0}, "mvar_reader", 0);
    }

    for (int i = 0; i < total; i++) {
        sys_wait(pids[i]);
    }

    sys_sem_close(MVAR_SEM_WRITE);
    sys_sem_close(MVAR_SEM_MUTEX);

    printf("MVar test complete. Final value: %d\n", mvar_value);
    return 0;
}

uint64_t cmd_clear(uint64_t argc, char *argv[]) {
    sys_clear();
    return 0;
}
