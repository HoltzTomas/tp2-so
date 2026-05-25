#include <stdint.h>
#include "lib.h"

void *memset(void *dest, int32_t c, uint64_t length) {
    uint8_t chr = (uint8_t)c;
    char *dst = (char *)dest;
    while (length--)
        *dst++ = chr;
    return dest;
}

void *memcpy(void *dest, const void *src, uint64_t length) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    while (length--)
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
