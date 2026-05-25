#ifndef INTERRUPTS_H
#define INTERRUPTS_H

void _cli(void);
void _sti(void);
void _hlt(void);
void pic_master_eoi(void);
void irq_init(void);
void irq_enable(void);
void irq_disable(void);
void force_timer(void);
void *_initialize_stack_frame(void (*func)(uint64_t, char **), void *args, void *stack_top);

#endif
