#ifndef VIDEO_H
#define VIDEO_H

#include <stdint.h>

void video_init(void);
void video_put_char(char c);
void video_put_string(const char *str);
void video_clear(void);
void video_set_cursor(uint32_t row, uint32_t col);
uint32_t video_get_row(void);
uint32_t video_get_col(void);

#endif
