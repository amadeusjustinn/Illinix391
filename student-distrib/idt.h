#ifndef IDT_H
#define IDT_H

#include "x86_desc.h"
#include "lib.h"
#include "idt_macro.h"
#include "system_call.h"
#include "system_call_linkage.h"

#define GATE_SIZE_32_BIT 1
#define KERNEL_DPL 0
#define USER_DPL 3
#define RESERVED3_INTERRUPT 0
#define RESERVED3_TRAP 1
#define IRQ_START 0x20
#define PIT_VECTOR 0x20
#define KEYBOARD_VECTOR 0x21
#define RTC_VECTOR 0x28
#define SYSTEM_CALL_VECTOR 0x80

// Prints the exception and freezes kernel
extern void blue_screen(char exp_name[]);

// Prints that system call occurred
extern void system_call(void);

// Functions for each of the exceptions from 0x00 to 0x13
extern void divide_by_zero_exp(void);
extern void debug_exp(void);
extern void non_maskable_interrupt_exp(void);
extern void breakpoint_exp(void);
extern void overflow_exp(void);
extern void bound_range_exceeded_exp(void);
extern void invalid_opcode_exp(void);
extern void device_not_available_exp(void);
extern void double_fault_exp(void);
extern void coprocessor_segment_overrun_exp(void);
extern void invalid_tss_exp(void);
extern void segment_not_present_exp(void);
extern void stack_segment_fault_exp(void);
extern void general_protection_fault_exp(void);
extern void page_fault_exp(void);
extern void x87_floating_point_exp(void);
extern void alignment_check_exp(void);
extern void machine_check_exp(void);
extern void simd_floating_point_exp(void);

// Initializes the IDT and sets up the descriptors for exceptions and system calls
extern void idt_init(void);

#endif
