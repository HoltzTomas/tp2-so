#include "lib.h"

void *memset(void *dest, int32_t c, uint64_t length) {
    uint8_t chr = (uint8_t)c;
    char *dst = (char *)dest;

    while (length--)
        dst[length] = chr;

    return dest;
}

void *memcpy(void *dest, const void *src, uint64_t length) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    for (uint64_t i = 0; i < length; i++)
        d[i] = s[i];

    return dest;
}

uint64_t strlen(const char *str) {
    uint64_t len = 0;
    while (str[len] != '\0')
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

char *strncpy(char *dest, const char *src, uint64_t n) {
    uint64_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';
    return dest;
}
