#include "paging.h"

// static uint32_t var_cr0, var_cr4;

/*
 * DESCRIPTION: Initializes page directory for kernel.
 *
 * INPUTS: none
 * 
 * OUTPUTS: none
 * 
 * SIDE EFFECTS: Initializes page directory.
 * 
 */

void page_directory_init(){
    // unless specified, section has ring 0 perms

    int i;

    //initialize page table
    for(i = 0; i < table_entries; i++){
        // page_table[i] = 0; 

        // first term is to find correct page, second to enable read/write
        page_table[i] = (i * FOUR_KB_SIZE) | READ_WRITE;
        page_directory[i] = READ_WRITE;
    }
    //initialize directory

    page_table[VIDEO_INDEX] |= USER | PRESENT;
    page_table[VIDEO_INDEX + 1] |= USER | PRESENT; // additional terminals
    page_table[VIDEO_INDEX + 2] |= USER | PRESENT;
    page_table[VIDEO_INDEX + 3] |= USER | PRESENT;

    //linking page table to page directory
    
    page_directory[0] = ((uint32_t)page_table) | READ_WRITE | PRESENT;

    //linking 4mb page to page directory
    
    page_directory[1] = FOUR_MB_SIZE | SIZE | READ_WRITE | PRESENT;

    //turn on 4 mb pages
    setPD(page_directory);
    enablePaging();

}

/*
 * DESCRIPTION: Initializes user page directory entry.
 *
 * INPUTS: addr - starting address 
 * 
 * OUTPUTS: none
 * 
 * SIDE EFFECTS: Initializes user page directory entry 
 * 
 */

void switch_pd(uint32_t addr) {


    // 0x7 --> 111, USER | READ WRITE | PRESENT
    page_directory[USER_PAGE] = addr | SIZE | USER | READ_WRITE | PRESENT;

    // flushes TLB
    flushTlb();

}

/*
 * DESCRIPTION: Maps video memory to correct physical address
 *
 * INPUTS: addr - beginning physical address, terminal_id - not implemented yet
 * 
 * OUTPUTS: none
 * 
 * SIDE EFFECTS: maps video to correct physical address
 * 
 */

void switch_vid() {

    // TODO: add multiterminal support
    // 0x7 --> 111, USER | READ WRITE | PRESENT
    // 4 MB * 33 = 132 MB, video memory starting address

    page_directory[USER_PAGE + 1] = (uint32_t)(uint32_t)page_table | 0x07;

    // 0x7 --> 111, USER | READ WRITE | PRESENT
    if (exec_terminal == disp_terminal) page_table[0] = ((uint32_t)VIDEO_ADDRESS) | 0x07;
    else page_table[0] = ((uint32_t)(VIDEO_ADDRESS + (exec_terminal + 1)*FOUR_KB_SIZE )) | 0x07;

    // flushes TLB
    asm volatile (
        "movl %%cr3, %%eax    \n"
        "movl %%eax, %%cr3    \n"
        :
        : 
        : "memory", "cc"

    );

}

/*
 * DESCRIPTION: Sets video memory to appropriate terminal
 *
 * INPUT: Current terminal number
 * 
 * OUTPUTS: none
 * 
 * SIDE EFFECTS: sets video memory
 *
 */
void set_vidmem(int32_t terminal_num) {
    if(disp_terminal == exec_terminal)

        page_table[VIDEO_INDEX] = ((uint32_t)VIDEO_ADDRESS) | USER | READ_WRITE | PRESENT;

    else

        page_table[VIDEO_INDEX] = ((uint32_t)(VIDEO_ADDRESS + (terminal_num+1)* FOUR_KB_SIZE )) | USER | READ_WRITE | PRESENT;

    flushTlb();   
}

/*
 * DESCRIPTION: restores vidmem to default settings
 *
 * INPUTS: none
 * 
 * OUTPUTS: none
 * 
 * SIDE EFFECTS: restores video memory
 *
 */
void restore_vidmem(void) {
    page_table[VIDEO_INDEX] = ((uint32_t)VIDEO_ADDRESS) | USER | READ_WRITE | PRESENT;
    flushTlb();
}

/*
 * DESCRIPTION: flushes the tlb
 *
 * INPUTS:
 * 
 * OUTPUTS:
 * 
 * SIDE EFFECTS: flushes the tlb
 * 
 */
void flushTlb() {
    asm volatile(
        "movl %%cr3, %%eax    \n"
        "movl %%eax, %%cr3    \n"
        :
        : 
        : "memory", "cc"
    );
}

/* remap_program
 * Inputs: PID number of process
 * Return Value: void
 * Function: Initialises page for user program
 */
void remap_program(uint32_t pid){

    uint32_t addr = EIGHT_MB_SIZE + pid * FOUR_MB_SIZE;

    // 32 = 128MB virtual 
    page_directory[32] =  addr | SIZE | USER | READ_WRITE | PRESENT;

    // flush TLB
    flushTlb();
    return;
}
