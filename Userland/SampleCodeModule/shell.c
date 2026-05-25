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
    int state;
    uint8_t foreground;
    int16_t parentPid;
} ProcInfo;

/* =============== Built-in commands as process functions =============== */

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

static void cat_func(uint64_t argc, char *argv[]) {
    (void)argc;
    (void)argv;
    char c;
    while (sys_read(0, &c, 1) > 0) {
        if (c == 4) /* Ctrl+D = EOF */
            break;
        sys_write(1, &c, 1);
    }
    sys_exit(0);
}

static void wc_func(uint64_t argc, char *argv[]) {
    (void)argc;
    (void)argv;
    uint64_t lines = 0, words = 0, chars = 0;
    char c;
    int in_word = 0;

    while (sys_read(0, &c, 1) > 0) {
        if (c == 4) /* EOF */
            break;
        chars++;
        if (c == '\n')
            lines++;
        if (c == ' ' || c == '\n' || c == '\t') {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            words++;
        }
    }
    printf("  %u %u %u\n", lines, words, chars);
    sys_exit(0);
}

static void filter_func(uint64_t argc, char *argv[]) {
    (void)argc;
    (void)argv;
    char c;
    while (sys_read(0, &c, 1) > 0) {
        if (c == 4) /* EOF */
            break;
        if (c >= 'a' && c <= 'z') {
            /* Convert to uppercase (filter vowels example) */
            if (c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u') {
                sys_write(1, &c, 1);
            }
        } else {
            sys_write(1, &c, 1);
        }
    }
    sys_exit(0);
}

/* =============== Shell helpers =============== */

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
    printf("  help              - Show this help\n");
    printf("  clear             - Clear screen\n");
    printf("  mem               - Show memory info\n");
    printf("  ps                - List processes\n");
    printf("  loop [&]          - Start loop process\n");
    printf("  kill <pid>        - Kill process\n");
    printf("  nice <pid> <prio> - Change priority (0-4)\n");
    printf("  block <pid>       - Block/unblock process\n");
    printf("  cat [&]           - Echo stdin to stdout\n");
    printf("  wc [&]            - Count lines/words/chars\n");
    printf("  filter [&]        - Remove vowels\n");
    printf("  Supports: cmd1 | cmd2, cmd &, Ctrl+C, Ctrl+D\n");
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
        int nameLen = (int)strlen(buf[i].name);
        for (int j = nameLen; j < 21; j++) putchar(' ');
        printf("%s", state_str(buf[i].state));
        int stLen = (int)strlen(state_str(buf[i].state));
        for (int j = stLen; j < 9; j++) putchar(' ');
        printf("%d     %d   %d\n", buf[i].priority, buf[i].foreground, buf[i].parentPid);
    }
}

typedef void (*CmdFunc)(uint64_t, char *[]);

static CmdFunc get_command_func(const char *name) {
    if (strcmp(name, "loop") == 0) return (CmdFunc)loop_func;
    if (strcmp(name, "cat") == 0) return (CmdFunc)cat_func;
    if (strcmp(name, "wc") == 0) return (CmdFunc)wc_func;
    if (strcmp(name, "filter") == 0) return (CmdFunc)filter_func;
    return NULL;
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

static int find_pipe(char *args[], int argc) {
    for (int i = 0; i < argc; i++) {
        if (strcmp(args[i], "|") == 0)
            return i;
    }
    return -1;
}

static void run_piped(char *args[], int argc, int pipe_pos, int background) {
    /* Left command: args[0..pipe_pos-1], Right command: args[pipe_pos+1..argc-1] */
    char *left_cmd = args[0];
    char *right_cmd = args[pipe_pos + 1];

    CmdFunc left_func = get_command_func(left_cmd);
    CmdFunc right_func = get_command_func(right_cmd);

    if (left_func == NULL) {
        printf("%s: command not found\n", left_cmd);
        return;
    }
    if (right_func == NULL) {
        printf("%s: command not found\n", right_cmd);
        return;
    }

    int pipeFds[2];
    if (sys_pipe_create(pipeFds) < 0) {
        printf("Error creating pipe\n");
        return;
    }

    /* Left process writes to pipe */
    /* We pack fds into extra: fg in low byte, and pass fds via separate syscall */
    uint8_t fg = background ? 0 : 1;
    int64_t leftPid = sys_create_process((void *)left_func, 0, NULL, left_cmd, fg);
    if (leftPid < 0) {
        printf("Error creating left process\n");
        return;
    }
    /* Set FDs: stdin=STDIN(0), stdout=pipeFds[1] (write end) */
    /* We need a syscall for this... for now the process inherits shell FDs */
    /* TODO: proper FD passing - processes read/write from their assigned FDs */

    int64_t rightPid = sys_create_process((void *)right_func, 0, NULL, right_cmd, fg);
    if (rightPid < 0) {
        printf("Error creating right process\n");
        sys_kill(leftPid);
        return;
    }

    /* For now, close pipe FDs from shell side */
    sys_pipe_close(pipeFds[0]);
    sys_pipe_close(pipeFds[1]);

    if (!background) {
        sys_wait(leftPid);
        sys_wait(rightPid);
    }
}

static void run_command(char *args[], int argc, int background) {
    CmdFunc func = get_command_func(args[0]);
    if (func == NULL) {
        /* Built-in non-process commands */
        if (strcmp(args[0], "help") == 0) {
            cmd_help();
        } else if (strcmp(args[0], "clear") == 0) {
            for (int i = 0; i < 25; i++) putchar('\n');
        } else if (strcmp(args[0], "mem") == 0) {
            cmd_mem();
        } else if (strcmp(args[0], "ps") == 0) {
            cmd_ps();
        } else if (strcmp(args[0], "kill") == 0) {
            if (argc < 2) { printf("Usage: kill <pid>\n"); return; }
            int pid = atoi(args[1]);
            if (pid <= 0) { printf("Cannot kill PID %d\n", pid); return; }
            if (sys_kill(pid) < 0)
                printf("Error killing PID %d\n", pid);
            else
                printf("Killed PID %d\n", pid);
        } else if (strcmp(args[0], "nice") == 0) {
            if (argc < 3) { printf("Usage: nice <pid> <prio>\n"); return; }
            int pid = atoi(args[1]);
            int prio = atoi(args[2]);
            if (sys_nice(pid, prio) < 0)
                printf("Error changing priority\n");
            else
                printf("PID %d priority set to %d\n", pid, prio);
        } else if (strcmp(args[0], "block") == 0) {
            if (argc < 2) { printf("Usage: block <pid>\n"); return; }
            int pid = atoi(args[1]);
            if (sys_block(pid) < 0)
                printf("Error blocking PID %d\n", pid);
            else
                printf("Toggled block on PID %d\n", pid);
        } else {
            printf("%s: command not found\n", args[0]);
        }
        return;
    }

    /* It's a process command */
    uint8_t fg = background ? 0 : 1;
    int64_t pid = sys_create_process((void *)func, (uint64_t)(argc - 1),
                                     argc > 1 ? &args[1] : NULL, args[0], fg);
    if (pid < 0) {
        printf("Error creating process\n");
        return;
    }
    if (background) {
        printf("[%d] %s\n", pid, args[0]);
    } else {
        sys_wait(pid);
    }
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

        int pipe_pos = find_pipe(args, argc);
        if (pipe_pos > 0) {
            run_piped(args, argc, pipe_pos, background);
        } else {
            run_command(args, argc, background);
        }
    }
}

int main(void) {
    shell_run();
    return 0;
}
