#include <stdint.h>
#include "include/shell.h"

extern char bss;
extern char endOfBinary;

int main(void);

int _start(void) {
    char *p = &bss;
    while (p < &endOfBinary)
        *p++ = 0;

    shell_run();
    return 0;
}
