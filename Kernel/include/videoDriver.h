#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#include <stdint.h>

void vd_init(void);
void vd_putchar(char c);
void vd_print(const char *str);
void vd_clear(void);

#endif
