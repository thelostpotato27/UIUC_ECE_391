/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void)
{
    outb(ICW1, MASTER_8259_PORT); // init ICW ports
    // add 1 for the data port
    outb(ICW2_MASTER, MASTER_8259_PORT + 1); // map IRQ0-7 to 0x20 to 0x27
    outb(ICW3_MASTER, MASTER_8259_PORT + 1); // map slave to IR2
    outb(ICW4, MASTER_8259_PORT + 1); // set 8086 mode

    outb(ICW1, SLAVE_8259_PORT); // init slave

    outb(ICW2_SLAVE, SLAVE_8259_PORT + 1); // vec offset for slave

    outb(ICW3_SLAVE, SLAVE_8259_PORT + 1); // tells slave it is at 2

    outb(ICW4, SLAVE_8259_PORT + 1); // set 8086 mode


    outb(master_mask, MASTER_8259_PORT + 1);
    outb(slave_mask, SLAVE_8259_PORT + 1);

    enable_irq(2); // port slave is connected on master
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num)
{
    if (irq_num > 15) // check invalid irq
        return;
    else if (irq_num < 8)
    {
        master_mask &= ~(1 << irq_num);
        // add 1 for the data port
        outb(master_mask, MASTER_8259_PORT + 1);
    }
    else
    {
        slave_mask &= ~(1 << (irq_num - 8));
        // add 1 for the data port
        outb(slave_mask, SLAVE_8259_PORT + 1);
    }
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num)
{
    if (irq_num > 15) // check invalid irq
        return;
    else if (irq_num < 8)
    {
        master_mask |= 1 << irq_num;
        // add 1 for the data port
        outb(master_mask, MASTER_8259_PORT + 1);
    }
    else
    {
        slave_mask |= 1 << (irq_num - 8);
        // add 1 for the data port
        outb(slave_mask, SLAVE_8259_PORT + 1);
    }
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num)
{
    if (irq_num > 15) // check invalid irq
        return;
    else if (irq_num < 8)
    {
        // add 1 for the data port
        outb(EOI | irq_num, MASTER_8259_PORT);
    }
    else
    {
        // add 1 for the data port
        outb(EOI | (irq_num - 8), SLAVE_8259_PORT);
        outb(EOI | 2, MASTER_8259_PORT);
    }
}

