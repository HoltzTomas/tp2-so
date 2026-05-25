#ifndef LIBC_H
#define LIBC_H

#include <stdint.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

void putchar(char c);
void printf(const char *fmt, ...);
int gets(char *buf, int max_len);
uint64_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
void *memset(void *dest, int c, uint64_t n);
int atoi(const char *str);

#endif
