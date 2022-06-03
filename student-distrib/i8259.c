/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
static uint8_t master_mask; /* IRQs 0-7  */
static uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC; Disables all interrupts on the PICs initially */
void i8259_init(void) {
    // Disable all interrupts for both PICs
    outb(PIC_MASK_ALL, MASTER_8259_DATA);
    outb(PIC_MASK_ALL, SLAVE_8259_DATA);
    // Mask is set when interrupt is off
    master_mask = PIC_MASK_ALL;
    slave_mask = PIC_MASK_ALL;
    // Starts initialization in cascade mode
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);
    // Sets the starting position in the IDT
    outb(ICW2_MASTER, MASTER_8259_DATA);
    outb(ICW2_SLAVE, SLAVE_8259_DATA);
    // Tells master that there is a slave at IRQ2 (0000 0100)
    outb(ICW3_MASTER, MASTER_8259_DATA);
    // Tells slave that it is attached to IRQ2 of master
    outb(ICW3_SLAVE, SLAVE_8259_DATA);
    // Send information about the environment
    outb(ICW4, MASTER_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA);
    // Enable IRQ for the slave 
    enable_irq(SLAVE_IRQ);
}

/* 
    Enable (unmask) the specified IRQ 
    Input: irq_num: IRQ to enable
*/
int32_t enable_irq(uint32_t irq_num) {
    // If IRQ is out of range, skip
    if (irq_num > 15 || irq_num < 0){
        return -1;
    }
    // Port value determining which PIC to set
    uint16_t port;
    // Value of the bit mask for the PIC; when a bit is set, PIC ignores its request
    uint8_t mask_value;
    // If irq is less than 8, it corresponds to master
    if (irq_num < NUM_IRQ){
        port = MASTER_8259_DATA;
        // Get the current mask value for the PIC and then clear the bit mask for the corresponding irq
        mask_value = master_mask & ~(1 << irq_num);
        master_mask = mask_value;
    }else{
        // Otherwise it corresponds to slave
        port = SLAVE_8259_DATA;
        // Offset by 8 to get slave's IRQ
        irq_num -= NUM_IRQ;
        // Get the current mask value for the PIC and then clear the bit mask for the corresponding irq
        mask_value = slave_mask & ~(1 << irq_num);
        slave_mask = mask_value;
    }
    // Output the new mask to the PIC
    outb(mask_value, port);
    return 0;
}

/* 
    Disable (mask) the specified IRQ 
    Input: irq_num: IRQ to disable
*/
int32_t disable_irq(uint32_t irq_num) {
    // If IRQ is out of range, skip
    if (irq_num > 15 || irq_num < 0){
        return -1;
    }
    // Port value determining which PIC to set
    uint16_t port;
    // Value of the bit mask for the PIC; when a bit is set, PIC ignores its request
    uint8_t mask_value;
    // If irq is less than 8, it corresponds to master
    if (irq_num < NUM_IRQ){
        port = MASTER_8259_DATA;
        // Get the current mask value for the PIC and then clear the bit mask for the corresponding irq
        mask_value = master_mask | (1 << irq_num);
        master_mask = mask_value;
    }else{
        // Otherwise it corresponds to slave
        port = SLAVE_8259_DATA;
        // Offset by 8 to get slave's IRQ
        irq_num -= NUM_IRQ;
        // Get the current mask value for the PIC and then clear the bit mask for the corresponding irq
        mask_value = slave_mask | (1 << irq_num);
        slave_mask = mask_value;
    }
    // Output the new mask to the PIC
    outb(mask_value, port);
    return 0;
}

/*
    Sends end of interrupt command to the PIC(s). If the interrupt that has finished came from master PIC, only
    send the EOI command to master. If it came from slave PIC, need to send command to both PICs
    Input: irq_num: IRQ value corresponding to the interrupt that has completed
*/
int32_t send_eoi(uint32_t irq_num) {
    // If IRQ is out of range, skip
    if (irq_num > 15 || irq_num < 0){
        return -1;
    }
    // If IRQ is larger than 8, corresponds to slave
    if (irq_num >= NUM_IRQ){
        // Offset by 8 to get slave's IRQ
        uint32_t slave_irq_num = irq_num - NUM_IRQ;
        // Send to both PICs
        outb(EOI | slave_irq_num, SLAVE_8259_PORT);
        outb(EOI | SLAVE_IRQ, MASTER_8259_PORT);
    }else{
        // If less than 8, send to master only
        outb(EOI | irq_num, MASTER_8259_PORT);
    }
    return 0;
}
