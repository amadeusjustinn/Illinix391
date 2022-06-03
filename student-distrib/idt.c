#include "idt.h"

/*
    Prints the name of the exception and freezes kernel; used by the other exception functions
    Input: exp_name: string indicating name of exception
*/
void blue_screen(int8_t exp_name[])
{
    // Outputs name of exception
    printf(exp_name);
    halt(256);
}

// Functions for each exception; calls blue_screen function with the name of the corresponding exception
void divide_by_zero_exp(void)
{
    blue_screen("Exception: divide by zero");
}

void debug_exp(void)
{
    blue_screen("Exception: debug");
}

void non_maskable_interrupt_exp(void)
{
    blue_screen("Exception: non-maskable interrupt");
}

void breakpoint_exp(void)
{
    blue_screen("Exception: breakpoint");
}

void overflow_exp(void)
{
    blue_screen("Exception: overflow");
}

void bound_range_exceeded_exp(void)
{
    blue_screen("Exception: bound range exceeded");
}

void invalid_opcode_exp(void)
{
    blue_screen("Exception: invalid opcode");
}

void device_not_available_exp(void)
{
    blue_screen("Exception: device not available");
}

void double_fault_exp(void)
{
    blue_screen("Exception: double fault");
}

void coprocessor_segment_overrun_exp(void)
{
    blue_screen("Exception: coprocessor segment overrun");
}

void invalid_tss_exp(void)
{
    blue_screen("Exception: invalid TSS");
}

void segment_not_present_exp(void)
{
    blue_screen("Exception: segment not present");
}

void stack_segment_fault_exp(void)
{
    blue_screen("Exception: stack-segment fault");
}

void general_protection_fault_exp(void)
{
    blue_screen("Exception: general protection fault");
}

void page_fault_exp(void)
{
    int CR2;
    int ESP;
    int EIP;
    int ERROR;
    asm volatile(
        "movl 8(%%esp), %2;"
        "movl 4(%%esp), %3;"
        "movl %%cr2, %0;"
        "movl %%esp, %1;"

        : "=r"(CR2), "=r"(ESP), "=r"(EIP), "=r"(ERROR));
    printf("CR2: %x ESP: %x \n", CR2, ESP);
    // printf("EIP: %x ERROR: %x \n", EIP, ERROR);
    blue_screen("Exception: page fault");
}

void x87_floating_point_exp(void)
{
    blue_screen("Exception: x87 floating-point exception");
}

void alignment_check_exp(void)
{
    blue_screen("Exception: alignment check");
}

void machine_check_exp(void)
{
    blue_screen("Exception: machine_check");
}

void simd_floating_point_exp(void)
{
    blue_screen("Exception: SIMD floating-point");
}

void idt_init(void)
{
    // Sets up the interrupt descriptor entries in the IDT
    unsigned int i;
    for (i = 0; i < NUM_VEC; i++)
    {
        // Segment selector corresponds to kernel CS
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        if (i < IRQ_START)
        {
            // If exception, correspond to trap gate
            idt[i].reserved3 = RESERVED3_TRAP;
        }
        else
        {
            // Everything else corresponds to interrupt gate
            idt[i].reserved3 = RESERVED3_INTERRUPT;
        }
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size = GATE_SIZE_32_BIT;
        idt[i].reserved0 = 0;
        if (i == SYSTEM_CALL_VECTOR)
        {
            // System calls have user privilege level
            idt[i].dpl = USER_DPL;
        }
        else
        {
            // Everything else has kernel
            idt[i].dpl = KERNEL_DPL;
        }
        idt[i].present = 1;
    }

    // Set the offsets for the exception descriptors
    SET_IDT_ENTRY(idt[0], divide_by_zero_exp);
    SET_IDT_ENTRY(idt[1], debug_exp);
    SET_IDT_ENTRY(idt[2], non_maskable_interrupt_exp);
    SET_IDT_ENTRY(idt[3], breakpoint_exp);
    SET_IDT_ENTRY(idt[4], overflow_exp);
    SET_IDT_ENTRY(idt[5], bound_range_exceeded_exp);
    SET_IDT_ENTRY(idt[6], invalid_opcode_exp);
    SET_IDT_ENTRY(idt[7], device_not_available_exp);
    SET_IDT_ENTRY(idt[8], double_fault_exp);
    SET_IDT_ENTRY(idt[9], coprocessor_segment_overrun_exp);
    SET_IDT_ENTRY(idt[10], invalid_tss_exp);
    SET_IDT_ENTRY(idt[11], segment_not_present_exp);
    SET_IDT_ENTRY(idt[12], stack_segment_fault_exp);
    SET_IDT_ENTRY(idt[13], general_protection_fault_exp);
    SET_IDT_ENTRY(idt[14], page_fault_exp);
    SET_IDT_ENTRY(idt[16], x87_floating_point_exp);
    SET_IDT_ENTRY(idt[17], alignment_check_exp);
    SET_IDT_ENTRY(idt[18], machine_check_exp);
    SET_IDT_ENTRY(idt[19], simd_floating_point_exp);

    // Set handler for keyboard in the IDT
    SET_IDT_ENTRY(idt[KEYBOARD_VECTOR], keyboard_handler_linkage);

    // Set handler for RTC in the IDT
    SET_IDT_ENTRY(idt[RTC_VECTOR], rtc_handler_linkage);

    // Set handler for PIT in the IDT
    SET_IDT_ENTRY(idt[PIT_VECTOR], pit_handler_linkage);

    // Set the offset for the system call descriptor
    SET_IDT_ENTRY(idt[SYSTEM_CALL_VECTOR], system_call_link);

    // Load the IDT
    lidt(idt_desc_ptr);
}
