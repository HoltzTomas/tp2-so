#include "include/shell.h"
#include "include/libc.h"
#include "include/syscalls.h"

#define MAX_CMD_LEN 256

static void cmd_help(void) {
    printf("Available commands:\n");
    printf("  help  - Show this help\n");
    printf("  clear - Clear screen\n");
    printf("  mem   - Show memory info\n");
    printf("\n");
}

static void cmd_mem(void) {
    uint64_t info[3];
    sys_mem_info(info);
    printf("Total: %u bytes\n", info[0]);
    printf("Used:  %u bytes\n", info[1]);
    printf("Free:  %u bytes\n", info[2]);
}

void shell_run(void) {
    char input[MAX_CMD_LEN];

    printf("Welcome to the OS Shell\n");
    printf("Type 'help' for available commands\n\n");

    while (1) {
        printf("$ ");
        int len = gets(input, MAX_CMD_LEN);
        putchar('\n');

        if (len <= 0)
            continue;

        if (strcmp(input, "help") == 0) {
            cmd_help();
        } else if (strcmp(input, "clear") == 0) {
            /* Clear is a syscall we'll add later - for now just print newlines */
            for (int i = 0; i < 25; i++)
                putchar('\n');
        } else if (strcmp(input, "mem") == 0) {
            cmd_mem();
        } else {
            printf("%s: command not found\n", input);
        }
    }
}

int main(void) {
    shell_run();
    return 0;
}
