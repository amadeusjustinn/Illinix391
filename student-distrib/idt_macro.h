#ifndef IDT_MACRO_H
#define IDT_MACRO_H

#include "keyboard.h"
#include "rtc.h"
#include "scheduling.h"

// Assembly linked handler for keyboard
extern void keyboard_handler_linkage(void);

// Assembly linked handler for RTC
extern void rtc_handler_linkage(void);

// Assembly linked handler for PIT
extern void pit_handler_linkage(void);

#endif
