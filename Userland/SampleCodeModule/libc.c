#include "include/libc.h"
#include "include/syscalls.h"
#include <stdarg.h>

int putchar(char c) {
    sys_write(STDOUT, &c, 1);
    return (int)c;
}

int puts(const char *str) {
    int len = 0;
    while (str[len])
        len++;
    sys_write(STDOUT, str, len);
    putchar('\n');
    return len + 1;
}

static void print_int(int64_t value) {
    char buffer[21];
    int i = 0;
    int negative = 0;

    if (value < 0) {
        negative = 1;
        value = -value;
    }
    if (value == 0) {
        putchar('0');
        return;
    }
    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }
    if (negative)
        putchar('-');
    while (--i >= 0)
        putchar(buffer[i]);
}

static void print_uint(uint64_t value) {
    char buffer[21];
    int i = 0;

    if (value == 0) {
        putchar('0');
        return;
    }
    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }
    while (--i >= 0)
        putchar(buffer[i]);
}

static void print_hex(uint64_t value) {
    char hex[] = "0123456789ABCDEF";
    char buffer[17];
    int i = 0;

    if (value == 0) {
        putchar('0');
        return;
    }
    while (value > 0) {
        buffer[i++] = hex[value & 0xF];
        value >>= 4;
    }
    while (--i >= 0)
        putchar(buffer[i]);
}

int printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int count = 0;

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
            case 'd': {
                int64_t val = va_arg(args, int64_t);
                print_int(val);
                break;
            }
            case 'u': {
                uint64_t val = va_arg(args, uint64_t);
                print_uint(val);
                break;
            }
            case 'x': {
                uint64_t val = va_arg(args, uint64_t);
                print_hex(val);
                break;
            }
            case 's': {
                const char *s = va_arg(args, const char *);
                if (s == 0)
                    s = "(null)";
                while (*s) {
                    putchar(*s++);
                    count++;
                }
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                putchar(c);
                count++;
                break;
            }
            case '%':
                putchar('%');
                count++;
                break;
            default:
                putchar('%');
                putchar(*fmt);
                count += 2;
                break;
            }
        } else {
            putchar(*fmt);
            count++;
        }
        fmt++;
    }
    va_end(args);
    return count;
}

char getchar(void) {
    char c;
    int n = sys_read(STDIN, &c, 1);
    if (n <= 0)
        return (char)EOF;
    return c;
}

int gets(char *buf, int size) {
    int i = 0;
    while (i < size - 1) {
        char c = getchar();
        if (c == (char)EOF || c == '\n') {
            break;
        }
        if (c == '\b') {
            if (i > 0) {
                i--;
                putchar('\b');
            }
            continue;
        }
        buf[i++] = c;
        putchar(c);
    }
    buf[i] = '\0';
    return i;
}

void *malloc(uint64_t size) {
    return sys_malloc(size);
}

void free(void *ptr) {
    sys_free(ptr);
}

void *memset(void *dest, int c, uint64_t n) {
    uint8_t *d = (uint8_t *)dest;
    while (n--)
        *d++ = (uint8_t)c;
    return dest;
}

void *memcpy(void *dest, const void *src, uint64_t n) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    while (n--)
        *d++ = *s++;
    return dest;
}

uint64_t strlen(const char *str) {
    uint64_t len = 0;
    while (str[len])
        len++;
    return len;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

char *strcpy(char *dest, const char *src) {
    char *ret = dest;
    while ((*dest++ = *src++))
        ;
    return ret;
}

char *strncpy(char *dest, const char *src, uint64_t n) {
    char *ret = dest;
    while (n && (*dest++ = *src++))
        n--;
    while (n--)
        *dest++ = '\0';
    return ret;
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

void itoa(int64_t value, char *buffer, int base) {
    char *ptr = buffer;
    char *ptr1 = buffer;
    char tmp;
    int negative = 0;

    if (value < 0 && base == 10) {
        negative = 1;
        value = -value;
    }

    do {
        int rem = value % base;
        *ptr++ = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        value /= base;
    } while (value);

    if (negative)
        *ptr++ = '-';
    *ptr-- = '\0';

    while (ptr1 < ptr) {
        tmp = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp;
    }
}
