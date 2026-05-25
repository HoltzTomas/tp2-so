#include "include/shell.h"
#include "include/libc.h"
#include "include/syscalls.h"

#define MAX_CMD_LEN 256
#define MAX_ARGS 16

typedef struct {
    uint64_t totalMemory;
    uint64_t usedMemory;
    uint64_t freeMemory;
} MemInfo;

typedef struct {
    int16_t pid;
    char name[32];
    uint8_t priority;
    int state; /* 0=READY,1=RUNNING,2=BLOCKED,3=ZOMBIE */
    uint8_t foreground;
    int16_t parentPid;
} ProcInfo;

static void loop_func(uint64_t argc, char *argv[]) {
    (void)argv;
    int64_t pid = sys_getpid();
    uint64_t delay = 1000000;
    if (argc > 0)
        delay = 500000;
    while (1) {
        printf("loop[%d] ", pid);
        for (uint64_t i = 0; i < delay; i++)
            ;
        sys_yield();
    }
}

static const char *state_str(int state) {
    switch (state) {
    case 0: return "READY";
    case 1: return "RUNNING";
    case 2: return "BLOCKED";
    case 3: return "ZOMBIE";
    default: return "?";
    }
}

static void cmd_help(void) {
    printf("Available commands:\n");
    printf("  help         - Show this help\n");
    printf("  clear        - Clear screen\n");
    printf("  mem          - Show memory info\n");
    printf("  ps           - List processes\n");
    printf("  loop [&]     - Start loop process\n");
    printf("  kill <pid>   - Kill process\n");
    printf("  nice <pid> <prio> - Change priority (0-4)\n");
    printf("  block <pid>  - Block/unblock process\n");
    printf("\n");
}

static void cmd_mem(void) {
    MemInfo info;
    sys_mem_info(&info);
    printf("Total: %u bytes (%u MB)\n", info.totalMemory, info.totalMemory / (1024 * 1024));
    printf("Used:  %u bytes\n", info.usedMemory);
    printf("Free:  %u bytes\n", info.freeMemory);
}

static void cmd_ps(void) {
    ProcInfo buf[64];
    int count = (int)sys_list_processes(buf, 64);
    printf("PID  NAME                 STATE    PRIO  FG  PARENT\n");
    printf("---  ----                 -----    ----  --  ------\n");
    for (int i = 0; i < count; i++) {
        printf("%d    %s", buf[i].pid, buf[i].name);
        /* Padding */
        int nameLen = 0;
        for (int j = 0; buf[i].name[j]; j++) nameLen++;
        for (int j = nameLen; j < 21; j++) putchar(' ');
        printf("%s", state_str(buf[i].state));
        int stLen = (int)strlen(state_str(buf[i].state));
        for (int j = stLen; j < 9; j++) putchar(' ');
        printf("%d     %d   %d\n", buf[i].priority, buf[i].foreground, buf[i].parentPid);
    }
}

static void cmd_loop(int background) {
    uint8_t fg = background ? 0 : 1;
    int64_t pid = sys_create_process((void *)loop_func, 0, NULL, "loop", fg);
    if (pid < 0) {
        printf("Error creating process\n");
        return;
    }
    printf("Created loop process PID=%d", pid);
    if (background)
        printf(" [background]");
    printf("\n");
    if (!background) {
        sys_wait(pid);
    }
}

static void cmd_kill(const char *arg) {
    if (arg == NULL || *arg == '\0') {
        printf("Usage: kill <pid>\n");
        return;
    }
    int pid = atoi(arg);
    if (pid <= 0) {
        printf("Cannot kill PID %d\n", pid);
        return;
    }
    int ret = (int)sys_kill(pid);
    if (ret < 0)
        printf("Error killing PID %d\n", pid);
    else
        printf("Killed PID %d\n", pid);
}

static void cmd_nice(const char *arg1, const char *arg2) {
    if (arg1 == NULL || arg2 == NULL || *arg1 == '\0' || *arg2 == '\0') {
        printf("Usage: nice <pid> <priority>\n");
        return;
    }
    int pid = atoi(arg1);
    int prio = atoi(arg2);
    int ret = (int)sys_nice(pid, prio);
    if (ret < 0)
        printf("Error changing priority\n");
    else
        printf("PID %d priority set to %d\n", pid, prio);
}

static void cmd_block(const char *arg) {
    if (arg == NULL || *arg == '\0') {
        printf("Usage: block <pid>\n");
        return;
    }
    int pid = atoi(arg);
    int ret = (int)sys_block(pid);
    if (ret < 0)
        printf("Error blocking PID %d\n", pid);
    else
        printf("Toggled block on PID %d\n", pid);
}

static int parse_args(char *input, char *args[], int max_args) {
    int argc = 0;
    int in_word = 0;

    while (*input && argc < max_args) {
        if (*input == ' ') {
            *input = '\0';
            in_word = 0;
        } else if (!in_word) {
            args[argc++] = input;
            in_word = 1;
        }
        input++;
    }
    return argc;
}

void shell_run(void) {
    char input[MAX_CMD_LEN];
    char *args[MAX_ARGS];

    printf("Welcome to the OS Shell\n");
    printf("Type 'help' for available commands\n\n");

    while (1) {
        printf("$ ");
        int len = gets(input, MAX_CMD_LEN);
        putchar('\n');

        if (len <= 0)
            continue;

        int argc = parse_args(input, args, MAX_ARGS);
        if (argc == 0)
            continue;

        int background = 0;
        if (argc > 1 && strcmp(args[argc - 1], "&") == 0) {
            background = 1;
            argc--;
        }

        if (strcmp(args[0], "help") == 0) {
            cmd_help();
        } else if (strcmp(args[0], "clear") == 0) {
            for (int i = 0; i < 25; i++)
                putchar('\n');
        } else if (strcmp(args[0], "mem") == 0) {
            cmd_mem();
        } else if (strcmp(args[0], "ps") == 0) {
            cmd_ps();
        } else if (strcmp(args[0], "loop") == 0) {
            cmd_loop(background);
        } else if (strcmp(args[0], "kill") == 0) {
            cmd_kill(argc > 1 ? args[1] : NULL);
        } else if (strcmp(args[0], "nice") == 0) {
            cmd_nice(argc > 1 ? args[1] : NULL, argc > 2 ? args[2] : NULL);
        } else if (strcmp(args[0], "block") == 0) {
            cmd_block(argc > 1 ? args[1] : NULL);
        } else {
            printf("%s: command not found\n", args[0]);
        }
    }
}

int main(void) {
    shell_run();
    return 0;
}
