#include "syscalls.h"

/*
 * DESCRIPTION: Halts the program and returns control to shell.
 *
 * INPUTS: The status of the halt (-1 for exception)
 * 
 * OUTPUTS: 0 (should never be reached)
 * 
 * SIDE EFFECTS: Halts all current processes and returns control to shell.
 * 
 */

int32_t halt(uint8_t status) {

    int i;

    cli();

    PCB_t* cur_pcb = terminals[exec_terminal].pcb;
    PCB_t* prev_pcb = cur_pcb->parent_pcb;

    //close old files
    for(i = 0;i < 8; i++){
        if(cur_pcb->open_files[i].flags == FLAG_BUSY) {

            close(i);
        }

        cur_pcb->open_files[i].file_op_table = null_fop;
    }

    if (terminals[exec_terminal].num_programs > 0) {
        process_flag[cur_pcb->pid] = 0;

        // closes a program in current terminal
        num_programs--;
        terminals[exec_terminal].num_programs--;
    }

    // check if the process to be halted is root shell process
    if (terminals[exec_terminal].num_programs == 0) {
        printf("Restarting with new shell...\n");
        // execute shell
        execute((uint8_t *)"shell");       
    } 

    // ---- restore parent paging -----------
    uint32_t addr = EIGHT_MB_SIZE + FOUR_MB_SIZE * (prev_pcb -> pid);

    switch_pd(addr);
    flushTlb();
    
    //restore tss_esp0
    tss.esp0 = cur_pcb->tss_esp0;

    terminals[exec_terminal].pcb = prev_pcb;

    sti();

    process_flag[cur_pcb->pid] = 0;

    asm volatile("movl %0, %%eax;"
                 "movl %1, %%esp;"
                 "movl %2, %%ebp;"
                 "jmp execute_return"
                : /* none */
                : "r"((uint32_t)status), "r"(cur_pcb->parent_esp), "r"(cur_pcb->parent_ebp)
                : "eax"
    );

    // should never return
    return 0;
}

/*
 * DESCRIPTION: Executes the given command
 *
 * INPUTS: a command/program to execute (ex: shell)
 * 
 * OUTPUTS: 0 upon success
 * 
 * SIDE EFFECTS: Executes the program, sets up new pcb, flushes TLB
 * 
 */
int32_t execute(const uint8_t* command) {

    cli();

    // counter
    int i;

    // local variables
    uint8_t parsed_cmd[MAX_CMD_LENGTH];
    uint8_t argv[MAX_ARGUMENT_NUM][MAX_ARGS];
    int32_t ret, num_args; 

    uint8_t ELF[4]; // used to check if executable
    int8_t filetype;
    dentry_t dentry;

    int32_t pid;

    uint8_t user_eip[4];
    uint32_t eip;

    uint32_t addr;

    // --------------- Parse arguments -------------------

    if (command == NULL) return -1; // NULL command

    if(strlen((int8_t *) command) == 1) return 0;
    if (command[0] == '\0' || command[0] == ' ') return -1; // invalid command

    ret = parse_cmd(command, parsed_cmd, argv);

    if (ret == -1) return -1; // invalid command name

    if (ret == -2) {
        asm volatile( "call halt;");
        // halt(0);
    }

    num_args = (ret > -1) ? ret : 0; 

    // -------------------- file type validation --------------------------
    
    if (num_programs >= 6) { // max program number
        printf("Maximum processes have been reached.\n");
        return 0;
    }
    
    filetype = read_dentry_by_name(parsed_cmd, &dentry);

    if (filetype == -1) return -1;

    // ensure ELF file -- read first 4 characters
    filetype = read_data(dentry.inode_num, 0, ELF, 4);

    if(filetype == -1) return -1;

    // 0x7F is DEL char - see MP3 checkpoint 3 writeup for details on characters to validate
    if (ELF[0] != 0x7F || ELF[1] != 'E' || ELF[2] != 'L' || ELF[3] != 'F') {
        return -1; // not executable
    }

    // -------------------- set up PID ---------------------------------

    int pid_full = 1;
    for (i = 0; i < 6; i++) {
        if (!(process_flag[i])) {
            process_flag[i] = 1;
            pid = i;
            pid_full = 0;
            break;
        }
    }

    if (pid_full) {
        printf("Maximum processes have been reached.\n");
        return 0;
    }

    //updates total program count
    num_programs++;
    terminals[exec_terminal].num_programs++;    

    //Bytes 24 to 27 of the executable. entry point
    read_data(dentry.inode_num, 24, user_eip, 4); // Read eip from elf (location 24)
    eip = *((uint32_t*)user_eip);

    //------------------------------ set up paging ----------------------------------
    
    addr = EIGHT_MB_SIZE + pid * FOUR_MB_SIZE;
    switch_pd(addr);
    flushTlb();

    // -------------------- loads program into correct location ------------------------
	
    read_data(dentry.inode_num, 0, (uint8_t *) USER_PROGRAM_ADDR, inode_start[dentry.inode_num].data_length);
    
    // -------------------- Set up PCB --------------
    
    uint32_t temp_ebp, temp_esp;

    // scheduler will set up shell if running on terminal first time
    if (terminals[exec_terminal].pcb != NULL && terminals[exec_terminal].pcb->scheduled) {
            
            // if already scheduled to run
            temp_esp = terminals[exec_terminal].pcb->user_esp;
            temp_ebp = terminals[exec_terminal].pcb->user_ebp;

            terminals[exec_terminal].pcb->scheduled = 0;
        }
    else {
            // needs to get values from scheduler
            temp_esp = 0;
            temp_ebp = 0;
        }
    
    PCB_t* pcb_ptr;
    pcb_ptr = (PCB_t *)(EIGHT_MB_SIZE - (pid + 1) * EIGHT_KB_SIZE); 
    pcb_init(pcb_ptr, parsed_cmd, argv, pid, num_args); 

    pcb_ptr->user_ebp = temp_ebp;
    pcb_ptr->user_esp = temp_esp;

    //save esp and ebp

        asm volatile("       \n\
            movl %%ebp, %0   \n\
            movl %%esp, %1   \n\
            "
            :"=r"(pcb_ptr->parent_ebp), "=r"(pcb_ptr->parent_esp) );

    terminals[exec_terminal].pcb = pcb_ptr;

    if (strncmp("shell", (int8_t*)parsed_cmd, 5) == 0) { // is this a shell?
        terminals[exec_terminal].pcb->is_shell = 1;
    } 

  //------------Prepare for context switch--------------------------------------------------

    //switching privilege level
    tss.ss0 = KERNEL_DS;
    tss.esp0 = EIGHT_MB_SIZE - (EIGHT_KB_SIZE * terminals[exec_terminal].pcb->pid);


    context_switch(eip); // Page fault here at stack

    asm volatile( "execute_return:" );

    return 0;
}

/*
 * DESCRIPTION: Reads from current file.
 *
 * INPUTS: fd -- file descriptor, buf -- buffer to be read to.
 * nbytes -- number of bytes to be read
 * 
 * OUTPUTS: 0 upon success
 * 
 * SIDE EFFECTS: reads into buffer
 * 
 */

int32_t read(int32_t fd, void* buf, int32_t nbytes) {
    int32_t ret = 0;
 
    if (valid_fd(fd) == -1 || fd == 1 // valid fd check; don't want STDOUT (fd of 1)
    || !buf || nbytes < 0 || terminals[exec_terminal].pcb->open_files[fd].flags != FLAG_BUSY) { 
        return -1;
    }
    
    // int32_t(*read_handler)(int32_t, void*, int32_t);
    
    // read_handler = (curr_pcb->open_files[fd].file_op_table[2]);
    ret = terminals[exec_terminal].pcb->open_files[fd].file_op_table.read(fd, buf, nbytes);

    return ret;
}

/*
 * DESCRIPTION: Calls the appropriate write function of the current file.
 *
 * INPUTS: fd -- file to be written to, buf -- buffer to be written to
 * (not applicable to regular file or rtc), nbytes -- number of bytes
 * 
 * OUTPUTS: 0 upon success
 * 
 * SIDE EFFECTS: writes to terminal or new frequency to RTC
 * 
 */

int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
    int32_t ret = 0;
 
    if (valid_fd(fd) == -1 || fd == 0 // valid fd check; don't want STDIN (fd of 0)
    || !buf || nbytes < 0 || terminals[exec_terminal].pcb->open_files[fd].flags != FLAG_BUSY) { 
        return -1;
    }
    
    ret = terminals[exec_terminal].pcb->open_files[fd].file_op_table.write(fd, buf, nbytes);

    return ret;
}

/*
 * DESCRIPTION: Opens file and adds to PCB.
 *
 * INPUTS: Name of the file.
 * 
 * OUTPUTS: 0 upon success.
 * 
 * SIDE EFFECTS: Adds file to PCB, initializes if necessary.
 * 
 */

int32_t open(const uint8_t* filename) {
    int i;
    int fd = -1; // default no spot found

    dentry_t dentry;

    if (read_dentry_by_name(filename, &dentry)) return -1; // invalid file

    if (dentry.filetype < 0 || dentry.filetype > 2) return -1; // invalid file type, should never happen

    PCB_t* pcb_ptr = terminals[exec_terminal].pcb;

    // User file starts at index 2 (0 is STDIN, 1 is STDOUT), maximum of 8 files in PCB
    for (i = 2; i < 8; i++) {

        if(pcb_ptr->open_files[i].flags == FLAG_FREE) {
            fd = i;
            pcb_ptr->open_files[i].file_pos = 0;
            pcb_ptr->open_files[i].inode_num = (dentry.filetype == 2)? dentry.inode_num  : 0;
            pcb_ptr->open_files[i].flags = FLAG_BUSY;

            if(dentry.filetype == 0){       //rtc file
                pcb_ptr->open_files[i].file_op_table = (fop_t)rtc_fop;
            }else if (dentry.filetype == 1){   //directory file
                pcb_ptr->open_files[i].file_op_table = (fop_t)dir_fop;
            }else if (dentry.filetype == 2){   //regular file
                pcb_ptr->open_files[i].file_op_table = (fop_t)filesys_fop;   //placeholder until fop is made
            }

            terminals[exec_terminal].pcb->open_files[fd].file_op_table.open(filename);

            return i; /* return fd upon success*/
        }
    }

    if (i == 8) // not found
        return -1; 

    return 0;
}

/*
 * DESCRIPTION: closes a file and marks it free
 *
 * INPUTS: fd of the file to be closed
 * 
 * OUTPUTS: 0 upon success, -1 upon failure
 * 
 * SIDE EFFECTS: closes file in curr_pcb
 * 
 */

int32_t close(int32_t fd) {

    if (valid_fd(fd) == -1 || fd < 2) return -1; // either invalid fd or attempting to close STDIN/OUT
    terminals[exec_terminal].pcb->open_files[fd].file_op_table.close(fd);

    // clear all entries in curr_pcb.open_files[fd] to 0
    // curr_pcb->open_files[fd].flags = FLAG_FREE;
    clear_fd(fd);

    return 0;
}

// Checkpoint 4 

/*
 * DESCRIPTION: Copies arguments from current pcb into buffer
 *
 * INPUTS: buf - buffer to be copied to, nbytes - number of bytes to be copied
 * 
 * OUTPUTS: 0 upon success
 * 
 * SIDE EFFECTS: populate buffer with arguments
 * 
 */
int32_t getargs(uint8_t* buf, int32_t nbytes) {

    uint8_t* argv = (uint8_t *) terminals[exec_terminal].pcb->argv;

    //If there are no arguments or invalid copy size
    if ((terminals[exec_terminal].pcb->argv[0][0] == '\0') || nbytes <= 0) return -1;

    // truncates argument if necessary
    nbytes = (nbytes > MAX_ARGS) ? MAX_ARGS : nbytes;

    memcpy(buf, argv, nbytes);

    return 0;
}

/*
 * DESCRIPTION: Remaps start of video memory to specified screen start address. 
 * Rejects invalid values, i.e. must fall within user-level space. A 4kB page is added
 * to hold video memory.
 *
 * INPUTS: screen_start -- beginning of video memory
 * 
 * OUTPUTS: 0 upon success
 * 
 * SIDE EFFECTS:  maps the text-mode video memory into user space at a pre-set virtual address
 */
int32_t vidmap(uint8_t** screen_start) {
    if (!screen_start) return -1;

    int addr = (uint32_t)screen_start; // converts argument into numerical address

    if (addr < VIDEO_START || addr > VIDEO_END) return -1;

    switch_vid();

    *screen_start = (uint8_t *)(VIDEO_END); // video memory start address (132 MB)

    return VIDEO_END;
}

// Extra Credit -- returns failure (-1) for now

int32_t set_handler(int32_t signum, void* handler_address) {
    return -1;
}

int32_t sigreturn(void) {
    return -1;
}

// /*
//  * DESCRIPTION: clears open files' file descriptors
//  *
//  * INPUTS: array of open files
//  * 
//  * OUTPUTS:
//  * 
//  * SIDE EFFECTS: clears open files
//  * 
//  */
// // TODO: implement ;-;
// void clearFd(PCB_entry_t* openFd) {
//     int idx = 0;

//     // going through all 8 possible entries in open_files
//     for (idx = 0; idx < 8; idx++) {
//         openFd[idx].flags = 0;
//     }
// }
