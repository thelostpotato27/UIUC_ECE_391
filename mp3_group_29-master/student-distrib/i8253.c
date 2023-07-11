#include "i8253.h"

/* i8253_init
 * 
 * DESCRIPTION: Initializes PIT. Code is taken from x86 assembly version
 * from https://wiki.osdev.org/Programmable_Interval_Timer. Here we
 * opt to set the frequency of interrupts to approximately one per
 * 10 ms, so the frequency conversion and validation code is not
 * needed.
 * 
 * INPUTS: none
 * 
 * OUTPUTS: none
 * 
 * SIDE EFFECTS: initializes i8253
 */
void i8253_init(void){
    int32_t freq = FREQUENCY; // sets frequency of interrupts to 10ms
    int32_t low_byte  = freq &  0xFF;    // only want lowest 8 bits
    int32_t high_byte = freq >> 8;       // only want highest 8 bits

    // Initializes PIT - selects command register
    // 0x30 - Channel 0, low byte/high bytes
    // 0x04 - Mode 4, software triggered strobe

    outb(PIT_PORT, CMD_REG);  

    outb(low_byte,  CHANNEL_0);
    outb(high_byte, CHANNEL_0);

    enable_irq(PIT_IRQ);
}

/* pit_handler
 *
 * INPUTS: none
 * 
 * OUTPUTS: none
 * 
 * SIDE EFFECTS: Switches between tasks in a round-robin fashion
 * at approximately every 10ms.
 */
void pit_handler(void)
{
    // TODO: round-robin scheduling... I cry
    
    // get current pcb
    PCB_t * curr_pcb = terminals[exec_terminal].pcb;
    PCB_t * next_pcb;

    // if no program currently running, execute shell
    if (!curr_pcb) {

        PCB_t pcb;

        //save esp and ebp
        asm volatile(
            "movl %%esp, %0       \n"
            "movl %%ebp, %1       \n"
            : "=r" (pcb.user_esp), "=r" (pcb.user_ebp)
            :
            : "memory"
        );

        pcb.scheduled = 1;

        // set executing terminal's pcb to this pcb
        terminals[exec_terminal].pcb = &pcb;

        // switch executing terminal to be displayed
        switch_terminal(exec_terminal);

        // for user to keep track of current terminal
        printf("Terminal %d\n", exec_terminal + 1);

        send_eoi(PIT_IRQ);

        // start shell
        execute((uint8_t *)"shell");
    }

    // save esp and ebp
    asm volatile(
        "movl %%esp, %0       \n"
        "movl %%ebp, %1       \n"
        : "=r" (curr_pcb->user_esp), "=r" (curr_pcb->user_ebp)
        :
        : "memory"
    ); 

    // switch terminal execution
    exec_terminal++;
    exec_terminal %= 3; // 0-2 are terminal numbers

    // return if no pcb -- will be handled in next interrupt
    if(!terminals[exec_terminal].pcb) {
        send_eoi(PIT_IRQ);
        return;
    } 

    // remaps and sets video memory in paging
    switch_vid();
    set_vidmem(exec_terminal);

    // gets pcb of process we're switching to
    next_pcb = terminals[exec_terminal].pcb;

    // switches paging
    uint32_t addr = EIGHT_MB_SIZE + FOUR_MB_SIZE * next_pcb->pid;
    switch_pd(addr); 

    // updates tss
    tss.ss0 = KERNEL_DS;
    tss.esp0 = EIGHT_MB_SIZE - EIGHT_KB_SIZE * (next_pcb->pid);

    send_eoi(PIT_IRQ);

    // save esp and ebp
    asm volatile(
        "movl %0, %%esp       \n"
        "movl %1, %%ebp       \n"
        :
        : "r" (next_pcb->user_esp), "r" (next_pcb->user_ebp)
        : "esp" , "ebp"
    );
}
