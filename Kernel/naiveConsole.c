#include "naiveConsole.h"

#define WIDTH 80
#define HEIGHT 25

static char *const video = (char *)0xB8000;
static uint32_t cursorX = 0;
static uint32_t cursorY = 0;
static const uint8_t color = 0x07;

static void ncSetChar(char c, uint32_t x, uint32_t y) {
    uint32_t pos = (y * WIDTH + x) * 2;
    video[pos] = c;
    video[pos + 1] = color;
}

void ncScrollUp(void) {
    uint32_t i;
    for (i = 0; i < (HEIGHT - 1) * WIDTH * 2; i++)
        video[i] = video[i + WIDTH * 2];
    for (i = (HEIGHT - 1) * WIDTH * 2; i < HEIGHT * WIDTH * 2; i += 2) {
        video[i] = ' ';
        video[i + 1] = color;
    }
}

void ncPrintChar(char character) {
    if (character == '\n') {
        cursorX = 0;
        cursorY++;
    } else if (character == '\b') {
        if (cursorX > 0) {
            cursorX--;
            ncSetChar(' ', cursorX, cursorY);
        }
    } else if (character == '\t') {
        cursorX = (cursorX + 4) & ~3;
    } else {
        ncSetChar(character, cursorX, cursorY);
        cursorX++;
    }

    if (cursorX >= WIDTH) {
        cursorX = 0;
        cursorY++;
    }
    if (cursorY >= HEIGHT) {
        ncScrollUp();
        cursorY = HEIGHT - 1;
    }
}

void ncPrint(const char *string) {
    while (*string)
        ncPrintChar(*string++);
}

void ncNewline(void) {
    ncPrintChar('\n');
}

void ncPrintDec(uint64_t value) {
    char buffer[20];
    int i = 0;
    if (value == 0) {
        ncPrintChar('0');
        return;
    }
    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }
    while (--i >= 0)
        ncPrintChar(buffer[i]);
}

void ncPrintHex(uint64_t value) {
    char buffer[17];
    int i;
    char hexChars[] = "0123456789ABCDEF";
    for (i = 15; i >= 0; i--) {
        buffer[i] = hexChars[value & 0xF];
        value >>= 4;
    }
    buffer[16] = 0;
    ncPrint(buffer);
}

void ncPrintBin(uint64_t value) {
    char buffer[65];
    int i;
    for (i = 63; i >= 0; i--) {
        buffer[i] = (value & 1) ? '1' : '0';
        value >>= 1;
    }
    buffer[64] = 0;
    ncPrint(buffer);
}

void ncClear(void) {
    uint32_t i;
    for (i = 0; i < HEIGHT * WIDTH * 2; i += 2) {
        video[i] = ' ';
        video[i + 1] = color;
    }
    cursorX = 0;
    cursorY = 0;
}

uint32_t ncGetCursorX(void) { return cursorX; }
uint32_t ncGetCursorY(void) { return cursorY; }
