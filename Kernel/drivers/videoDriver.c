#include "videoDriver.h"
#include "naiveConsole.h"

void vd_init(void) {
    ncClear();
}

void vd_putchar(char c) {
    ncPrintChar(c);
}

void vd_print(const char *str) {
    ncPrint(str);
}

void vd_clear(void) {
    ncClear();
}
