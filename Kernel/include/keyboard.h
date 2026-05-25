#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

void keyboard_init(void);
char keyboard_get_char(void);
int keyboard_has_char(void);
void keyboard_add_char(uint8_t scancode);
void keyboard_send_eof(void);
void keyboard_ctrl_c(void);

#endif
