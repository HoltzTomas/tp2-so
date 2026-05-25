#ifndef NAIVE_CONSOLE_H
#define NAIVE_CONSOLE_H

#include <stdint.h>

void ncPrint(const char *string);
void ncPrintChar(char character);
void ncNewline(void);
void ncPrintDec(uint64_t value);
void ncPrintHex(uint64_t value);
void ncPrintBin(uint64_t value);
void ncClear(void);
void ncScrollUp(void);
uint32_t ncGetCursorX(void);
uint32_t ncGetCursorY(void);

#endif
