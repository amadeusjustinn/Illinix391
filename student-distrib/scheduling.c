// References: https://wiki.osdev.org/Programmable_Interval_Timer
//             http://www.osdever.net/bkerndev/Docs/pit.htm

#include "scheduling.h"

// The data rate is actually a 'divisor' register for this device. The timer
// will divide it's input clock of 1.19MHz (1193180Hz) by the number you give it in
// the data register to figure out how many times per second to fire the signal for that channel

/* pit_init
 *
 *  Input: none
 *  Output: none
 *  Description: sets up the PIT
 */
void pit_init(void)
{
    outb(CMD_BYTE, CMD_REG);           /* Set our command byte 0x36 */
    outb(DIVISOR & MASK, CHANNEL_0);   /* Set low byte of divisor */
    outb(DIVISOR >> EIGHT, CHANNEL_0); /* Set high byte of divisor */
    current_terminal_run = 0;
    enable_irq(TIMER_IRQ);
    return;
}

void pit_handler(void)
{
    send_eoi(TIMER_IRQ);
    scheduler();
}

void scheduler(void)
{
    cli();
    // Turn on shell 0 if not on
    if (terminals[0].current_pid == -1)
    {
        execute_base_shell(0);
        return;
    }
    pcb_t *prev_mem_ptr = (pcb_t *)(BOTTOM_KERNEL - (PROCESS_SIZE * (terminals[current_terminal_run].current_pid + 1)));
    // Save esp
    register uint32_t saved_esp asm("esp");
    prev_mem_ptr->saved_esp = saved_esp;
    // Save ebp
    register uint32_t saved_ebp asm("ebp");
    prev_mem_ptr->saved_ebp = saved_ebp;

    // Work on next process
    current_terminal_run++;
    current_terminal_run = current_terminal_run % NUM_SHELLS;

    // Load video data into video memory directly
    if (current_terminal_run == current_terminal_view)
    {
        video_mem = (char *)VIDEO;
        // For fish
        if (page_table2[0].present == 1)
        {
            page_table2[0].bits_31_12 = VIDEO_12;
            flush_TLB();
        }
    }
    else
    {
        // Load video data into terminal video buffer
        video_mem = (char *)terminal_address[current_terminal_run];
        // For fish
        if (page_table2[0].present == 1)
        {
            page_table2[0].bits_31_12 = terminal_address[current_terminal_run];
            flush_TLB();
        }
    }

    // Base shell not active so execute it
    if (terminals[current_terminal_run].current_pid == -1)
    {
        video_mem = (char *)terminal_address[current_terminal_run];
        execute_base_shell(current_terminal_run);
        return;
    }

    // Get PCB for next process
    pcb_t *cur_mem_ptr = (pcb_t *)(BOTTOM_KERNEL - (PROCESS_SIZE * (terminals[current_terminal_run].current_pid + 1)));

    // Set up paging
    map(VIRTUAL_ADDR, BOTTOM_KERNEL + terminals[current_terminal_run].current_pid * FOUR_MB);

    // Start context switch to user mode
    tss.ss0 = KERNEL_DS;
    tss.esp0 = BOTTOM_KERNEL - (PROCESS_SIZE * (terminals[current_terminal_run].current_pid + 1));
    // Switch ESP/EBP
    asm volatile("                              \n\
            movl %0, %%esp                      \n\
            movl %1, %%ebp                      \n\
            "
                 :
                 : "r"(cur_mem_ptr->saved_esp), "r"(cur_mem_ptr->saved_ebp)
                 : "esp", "ebp");
    sti();
    return;
}
