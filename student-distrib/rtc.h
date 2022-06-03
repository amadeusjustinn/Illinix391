#ifndef RTC_H
#define RTC_H

#include "i8259.h"
#include "lib.h"
#include "keyboard.h"

#define RTC_IRQ 8 // IRQ8: Interrupt port where RTC generates clock ticks

#define RTC_INDEX_PORT 0x70 // Used to specify which register of the CMOS to write
#define RTC_RW_PORT 0x71    // Used to read or write to that register
#define NMI_OFF 0x80        // Used to set Bit 7 for disabling non-maskable interrupts
#define NMI_ON 0x7F         // Used to clear Bit 7 for enabling non-maskable interrupts
#define REG_B_OFFSET 0x0B   // Offset to get register B in CMOS
#define REG_C_OFFSET 0x0C   // Offset to get register C in CMOS
#define ENABLE_IRQ8 0x40    // Used to set bit 6 of register B to turn on periodic interrupt of RTC

#define FREQ_UPPER_LIMIT 1024                    // Highest valid interrupt frequency in Hz (2^10)
#define FREQ_LOWER_LIMIT 2                       // Lowest valid interrupt frequency in Hz (2^1)
#define FREQ_INIT FREQ_LOWER_LIMIT               // Lowest valid interrupt frequency in Hz (2^1)
#define FACTOR_INIT FREQ_UPPER_LIMIT / FREQ_INIT // Interrupt count for 2 Hz RTC (initialised at rtc_open)

/* Initializes the RTC to enable periodic interrupts (IRQ 8) */
extern void rtc_init(void);

/* Handler called when interrupt occurs */
extern void rtc_handler(void);

/* Initialises RTC frequency to 2 Hz */
extern int32_t rtc_open(const uint8_t *filename);

/* Unsets global variables that were set for virtualisation */
extern int32_t rtc_close(int32_t fd);

/* Blocks until the next interrupt */
extern int32_t rtc_read(int32_t fd, void *buf, int32_t nbytes);

/* Changes frequency to frequency stored in pointer */
extern int32_t rtc_write(int32_t fd, const void *buf, int32_t nbytes);

#endif
