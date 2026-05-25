#include "video.h"

static uint8_t *const VGA_BASE = (uint8_t *)0xB8000;
static const uint32_t WIDTH = 80;
static const uint32_t HEIGHT = 25;
static const uint8_t DEFAULT_ATTR = 0x07;

static uint32_t cursor_row = 0;
static uint32_t cursor_col = 0;

static void scroll(void) {
    for (uint32_t i = 0; i < (HEIGHT - 1) * WIDTH; i++) {
        VGA_BASE[i * 2] = VGA_BASE[(i + WIDTH) * 2];
        VGA_BASE[i * 2 + 1] = VGA_BASE[(i + WIDTH) * 2 + 1];
    }
    for (uint32_t i = (HEIGHT - 1) * WIDTH; i < HEIGHT * WIDTH; i++) {
        VGA_BASE[i * 2] = ' ';
        VGA_BASE[i * 2 + 1] = DEFAULT_ATTR;
    }
}

void video_init(void) {
    cursor_row = 0;
    cursor_col = 0;
    video_clear();
}

void video_put_char(char c) {
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
    } else if (c == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
            uint32_t pos = cursor_row * WIDTH + cursor_col;
            VGA_BASE[pos * 2] = ' ';
            VGA_BASE[pos * 2 + 1] = DEFAULT_ATTR;
        }
    } else if (c == '\t') {
        uint32_t spaces = 4 - (cursor_col % 4);
        for (uint32_t i = 0; i < spaces; i++)
            video_put_char(' ');
        return;
    } else {
        uint32_t pos = cursor_row * WIDTH + cursor_col;
        VGA_BASE[pos * 2] = c;
        VGA_BASE[pos * 2 + 1] = DEFAULT_ATTR;
        cursor_col++;
    }

    if (cursor_col >= WIDTH) {
        cursor_col = 0;
        cursor_row++;
    }

    if (cursor_row >= HEIGHT) {
        scroll();
        cursor_row = HEIGHT - 1;
    }
}

void video_put_string(const char *str) {
    while (*str)
        video_put_char(*str++);
}

void video_clear(void) {
    for (uint32_t i = 0; i < HEIGHT * WIDTH; i++) {
        VGA_BASE[i * 2] = ' ';
        VGA_BASE[i * 2 + 1] = DEFAULT_ATTR;
    }
    cursor_row = 0;
    cursor_col = 0;
}

void video_set_cursor(uint32_t row, uint32_t col) {
    cursor_row = row;
    cursor_col = col;
}

uint32_t video_get_row(void) {
    return cursor_row;
}

uint32_t video_get_col(void) {
    return cursor_col;
}
