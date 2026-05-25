#ifndef LIBC_H
#define LIBC_H

#include <stdint.h>
#include <stddef.h>

#define STDIN 0
#define STDOUT 1
#define EOF (-1)

typedef int pid_t;

int putchar(char c);
int puts(const char *str);
int printf(const char *fmt, ...);
char getchar(void);
int gets(char *buf, int size);

void *malloc(uint64_t size);
void free(void *ptr);
void *memset(void *dest, int c, uint64_t n);
void *memcpy(void *dest, const void *src, uint64_t n);
uint64_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, uint64_t n);
int atoi(const char *str);
void itoa(int64_t value, char *buffer, int base);

#endif
