#include "rtc.h"

// Referenced https://wiki.osdev.org/RTC

/**
 * @brief Initializes the RTC to enable periodic interrupts (IRQ 8)
 *
 *  Input: none
 *  Output: none
 * 
 * @note  IRQ handler must be present in IDT table before calling this
 *            since the interrupt will happen immediately
 * @note  Will switch on IRQ with default 1024 Hz rate
 */
void rtc_init(void)
{
    // Disable interrupts
    cli();
    // Select register B and disable NMI
    outb(NMI_OFF | REG_B_OFFSET, RTC_INDEX_PORT);
    // Read the current value of register B
    char prev_val = inb(RTC_RW_PORT);
    // Set index to register B again since a read to CMOS resets the index to register D
    outb(NMI_OFF | REG_B_OFFSET, RTC_INDEX_PORT);
    // Turn on bit 6 of register B
    outb(prev_val | ENABLE_IRQ8, RTC_RW_PORT);
    // Enable NMI
    outb(NMI_ON & REG_B_OFFSET, RTC_INDEX_PORT);
    // Enable interrupts for IRQ8 on PIC
    enable_irq(RTC_IRQ);
}

/**
 * @brief Handler called when interrupt occurs
 *            Reads value in register C and throws it away
 *  Input: none
 *  Output: none
 * 
 * @note  If register C is not read, another interrupt will not occur
 */
void rtc_handler(void)
{
    // interrupt_occurred = true;
    terminals[current_terminal_run].interrupt_count++;
    // Select register C
    outb(REG_C_OFFSET, RTC_INDEX_PORT);
    // Read the current value in register C
    inb(RTC_RW_PORT);
    // Called to test RTC interrupt
    // test_interrupts();
    // Send end-of-interrupt to notify PIC that it is done
    send_eoi(RTC_IRQ);
}

// open, write, loop read until interrupt count == factor, then set interrupt count = 0
// share count, factor for virtualisation

/**
 * @brief  Initialises RTC frequency to 2 Hz
 * 
 *  Input: filename
 *  Output: 0
 * 
 * @return 0 upon success
 */
int32_t rtc_open (const uint8_t* filename)
{
    // 2 Hz * ?? = 1024 Hz
    terminals[current_terminal_run].factor = FACTOR_INIT;
    // Set count to 0 for first batch of interrupts
    terminals[current_terminal_run].interrupt_count = 0;

    return 0;
}

/**
 * @brief  Unset global variables that were set for virtualisation
 *
 *  Input: fd
 *  Output: 0
 *
 * @return 0 upon success
 */
int32_t rtc_close (int32_t fd)
{
    terminals[current_terminal_run].factor = 0;
    terminals[current_terminal_run].interrupt_count = 0;
    return 0;
}

/**
 * @brief  Blocks until the next interrupt
 *
 *  Input: fd, buf, nbytes
 *  Output: 0
 * 
 * @return 0 when the next interrupt has occurred
 */
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes)
{
    // Wait until handler has been called an appropriate number of times
    while (terminals[current_terminal_run].interrupt_count < terminals[current_terminal_run].factor)
    {
    }
    // Reset count for next batch of interrupts
    terminals[current_terminal_run].interrupt_count = 0;

    return 0;
}

/**
 * @brief     Changes frequency to frequency stored in pointer
 *
 *  Input: fd, buf, nbytes
 *  Output: 0, -1
 * 
 * @param buf Pointer referencing address containing desired frequency
 * @return    0 upon success, -1 upon failure (invalid frequency)
 */
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes)
{
    // Acquire frequency of program by dereferencing buf
    uint32_t *buf_int32 = (uint32_t *)buf;
    uint32_t frequency = *buf_int32;

    // Return -1 if frequency is not a power of 2 or out of range [2, 1024]
    if ((frequency & (~(frequency - 1))) != frequency || frequency < FREQ_LOWER_LIMIT || frequency > FREQ_UPPER_LIMIT)
        return -1;

    // frequency * ?? = 1024 Hz
    terminals[current_terminal_run].factor = FREQ_UPPER_LIMIT / frequency;

    return 0;
}
