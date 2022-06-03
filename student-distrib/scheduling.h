#ifndef SCHEDULING_H
#define SCHEDULING_H

#include "lib.h"
#include "i8259.h"
#include "system_call.h"

#define CHANNEL_0 0x40
#define CMD_BYTE  0x36
#define CMD_REG   0x43
#define DIVISOR   11932 // 1193180 / 100 hz = 11932 
#define MASK      0xFF
#define EIGHT     8
#define TIMER_IRQ 0
#define NUM_SHELLS 3

int32_t current_terminal_run;

void pit_init(void);
uint32_t read_pit_count(void);
void pit_handler(void);
void scheduler(void);

#endif
