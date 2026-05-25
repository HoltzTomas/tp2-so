#include "include/shell.h"
#include "include/libc.h"
#include "include/syscalls.h"
#include "include/commands.h"
#include "include/test_util.h"

#define MAX_CMD_LEN 256
#define MAX_ARGS 16

typedef uint64_t (*CommandFunc)(uint64_t argc, char *argv[]);

typedef struct {
    const char *name;
    CommandFunc func;
} Command;

static Command commands[] = {
    {"help", cmd_help},
    {"mem", cmd_mem},
    {"ps", cmd_ps},
    {"loop", cmd_loop},
    {"kill", cmd_kill},
    {"nice", cmd_nice},
    {"block", cmd_block},
    {"cat", cmd_cat},
    {"wc", cmd_wc},
    {"filter", cmd_filter},
    {"mvar", cmd_mvar},
    {"clear", cmd_clear},
    {"test_mm", test_mm},
    {"test_proc", test_processes},
    {"test_prio", test_prio},
    {"test_sync", test_sync},
    {0, 0}};

static int parse_command(char *input, char *argv[]) {
    int argc = 0;
    int in_word = 0;

    while (*input && argc < MAX_ARGS - 1) {
        if (*input == ' ' || *input == '\t') {
            if (in_word) {
                *input = '\0';
                in_word = 0;
            }
        } else {
            if (!in_word) {
                argv[argc++] = input;
                in_word = 1;
            }
        }
        input++;
    }
    argv[argc] = 0;
    return argc;
}

static CommandFunc find_command(const char *name) {
    for (int i = 0; commands[i].name != 0; i++) {
        if (strcmp(name, commands[i].name) == 0)
            return commands[i].func;
    }
    return 0;
}

static int check_pipe(char *input, char *left, char *right) {
    int i = 0;
    while (input[i]) {
        if (input[i] == '|') {
            left[i] = '\0';
            int j = i + 1;
            while (input[j] == ' ')
                j++;
            int k = 0;
            while (input[j]) {
                right[k++] = input[j++];
            }
            right[k] = '\0';
            return 1;
        }
        left[i] = input[i];
        i++;
    }
    left[i] = '\0';
    return 0;
}

static int check_background(char *input) {
    int len = strlen(input);
    while (len > 0 && input[len - 1] == ' ')
        len--;
    if (len > 0 && input[len - 1] == '&') {
        input[len - 1] = '\0';
        while (len > 1 && input[len - 2] == ' ') {
            input[len - 2] = '\0';
            len--;
        }
        return 1;
    }
    return 0;
}

void shell_run(void) {
    char input[MAX_CMD_LEN];
    char left[MAX_CMD_LEN];
    char right[MAX_CMD_LEN];

    printf("Welcome to the OS Shell\n");
    printf("Type 'help' for available commands\n\n");

    while (1) {
        printf("$ ");
        int len = gets(input, MAX_CMD_LEN);
        putchar('\n');

        if (len == 0)
            continue;

        int background = check_background(input);

        if (check_pipe(input, left, right)) {
            char *argvL[MAX_ARGS];
            char *argvR[MAX_ARGS];
            int argcL = parse_command(left, argvL);
            int argcR = parse_command(right, argvR);

            if (argcL == 0 || argcR == 0) {
                printf("Invalid pipe command\n");
                continue;
            }

            CommandFunc funcL = find_command(argvL[0]);
            CommandFunc funcR = find_command(argvR[0]);

            if (funcL == 0) {
                printf("%s: command not found\n", argvL[0]);
                continue;
            }
            if (funcR == 0) {
                printf("%s: command not found\n", argvR[0]);
                continue;
            }

            int pipeFds[2];
            sys_pipe_create(pipeFds);

            int64_t pidL = sys_create_process((uint64_t)funcL, argcL - 1, argvL + 1, argvL[0], !background);
            int64_t pidR = sys_create_process((uint64_t)funcR, argcR - 1, argvR + 1, argvR[0], !background);

            if (!background) {
                sys_wait(pidL);
                sys_wait(pidR);
            }

            sys_pipe_close(pipeFds[0]);
            sys_pipe_close(pipeFds[1]);
        } else {
            char *argv[MAX_ARGS];
            int argc = parse_command(input, argv);

            if (argc == 0)
                continue;

            CommandFunc func = find_command(argv[0]);
            if (func == 0) {
                printf("%s: command not found\n", argv[0]);
                continue;
            }

            int64_t pid = sys_create_process((uint64_t)func, argc - 1, argv + 1, argv[0], !background);
            if (pid < 0) {
                printf("Error creating process\n");
                continue;
            }

            if (!background) {
                sys_wait(pid);
            }
        }
    }
}
