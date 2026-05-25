#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

void irq_init(void);
void irq_enable(void);
void irq_disable(void);

void pic_master_eoi(void);
void pic_slave_eoi(void);

uint8_t keyboard_handler(void);
void timer_handler(void);

uint64_t *schedule(uint64_t *rsp);

void _cli(void);
void _sti(void);
void _hlt(void);
void force_timer(void);

#endif
