#ifndef LIB_H
#define LIB_H

#include <stdint.h>

char *cpuVendor(char *result);
void *memset(void *dest, int32_t c, uint64_t length);
void *memcpy(void *dest, const void *src, uint64_t length);
uint64_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, uint64_t n);

#endif
