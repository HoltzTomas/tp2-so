#include "include/libc.h"
#include "include/syscalls.h"
#include <stdarg.h>

#define STDIN 0
#define STDOUT 1

void putchar(char c) {
    sys_write(STDOUT, &c, 1);
}

static void print_string(const char *s) {
    while (*s)
        putchar(*s++);
}

static void print_dec(int64_t value) {
    if (value < 0) {
        putchar('-');
        value = -value;
    }
    char buf[21];
    int i = 0;
    if (value == 0) {
        putchar('0');
        return;
    }
    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }
    while (i > 0)
        putchar(buf[--i]);
}

static void print_uint(uint64_t value) {
    char buf[21];
    int i = 0;
    if (value == 0) {
        putchar('0');
        return;
    }
    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }
    while (i > 0)
        putchar(buf[--i]);
}

static void print_hex(uint64_t value) {
    char buf[17];
    int i = 0;
    if (value == 0) {
        putchar('0');
        return;
    }
    while (value > 0) {
        int digit = value % 16;
        buf[i++] = (digit < 10) ? '0' + digit : 'A' + digit - 10;
        value /= 16;
    }
    while (i > 0)
        putchar(buf[--i]);
}

void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
            case 's':
                print_string(va_arg(args, const char *));
                break;
            case 'd':
                print_dec(va_arg(args, int64_t));
                break;
            case 'u':
                print_uint(va_arg(args, uint64_t));
                break;
            case 'x':
                print_hex(va_arg(args, uint64_t));
                break;
            case 'c':
                putchar((char)va_arg(args, int));
                break;
            case '%':
                putchar('%');
                break;
            default:
                putchar('%');
                putchar(*fmt);
                break;
            }
        } else {
            putchar(*fmt);
        }
        fmt++;
    }

    va_end(args);
}

int gets(char *buf, int max_len) {
    int i = 0;
    while (i < max_len - 1) {
        char c;
        sys_read(STDIN, &c, 1);

        if (c == '\n' || c == '\r') {
            break;
        } else if (c == '\b') {
            if (i > 0) {
                i--;
                putchar('\b');
            }
        } else if (c == 0x04) {
            /* Ctrl+D - EOF */
            if (i == 0)
                return -1;
            break;
        } else if (c == 0x03) {
            /* Ctrl+C */
            return -2;
        } else {
            buf[i++] = c;
            putchar(c);
        }
    }
    buf[i] = '\0';
    return i;
}

uint64_t strlen(const char *str) {
    uint64_t len = 0;
    while (str[len])
        len++;
    return len;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++))
        ;
    return dest;
}

void *memset(void *dest, int c, uint64_t n) {
    uint8_t *d = (uint8_t *)dest;
    while (n--)
        *d++ = (uint8_t)c;
    return dest;
}

int atoi(const char *str) {
    int result = 0;
    int sign = 1;
    if (*str == '-') {
        sign = -1;
        str++;
    }
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result * sign;
}
