#include "syscall_help.h"

// device jump tables
fop_t null_fop = {null_open, null_close, null_read, null_write};
fop_t stdin_fop = {terminal_open, terminal_close, terminal_read, terminal_write};
fop_t stdout_fop = {terminal_open, terminal_close, terminal_read, terminal_write};
fop_t dir_fop = {dir_open, dir_close, dir_read, dir_write};
fop_t filesys_fop = {file_open, file_close, file_read, file_write};
fop_t rtc_fop = {rtc_open, rtc_close, rtc_read, rtc_write};

// Initializes global variables to default values
void init_vars(void) {
    int i;

    cur_pid = 0;
    //parent_pid = 0;
    // process at pid

    // determines which pid is free -- up to 6 processes
    for (i = 0; i < 6; i++) process_flag[i] = 0;
    // global_status = 255;
    num_programs = 0;
    exec_terminal = 1;
    disp_terminal = 0;

}

/*
 * DESCRIPTION: Determines if given fd is present and possible
 *
 * INPUTS: fd -- file descriptor in current PCB
 * 
 * OUTPUTS: 0 for valid, -1 for invalid
 * 
 * SIDE EFFECTS: none
 * 
 */

int32_t valid_fd(int32_t fd) {
    if (fd >= 0 && fd < 8) { // up to 8 processes

        // TODO: ADD CHECK HERE TO ENSURE PRESENT PROCESS
        if (terminals[exec_terminal].pcb->open_files[fd].flags != FLAG_BUSY) return -1;

        return 0;
    }

    return -1;
}

/*
 * DESCRIPTION: Clears entry at the given fd
 *
 * INPUTS: fd -- entry in executing terminal to be cleared
 * 
 * OUTPUTS: none
 * 
 * SIDE EFFECTS: clears entry
 */
void clear_fd(int32_t fd) {

    // clears file operation table
    terminals[exec_terminal].pcb->open_files[fd].file_op_table.open = 0;
    terminals[exec_terminal].pcb->open_files[fd].file_op_table.close = 0;
    terminals[exec_terminal].pcb->open_files[fd].file_op_table.read = 0;
    terminals[exec_terminal].pcb->open_files[fd].file_op_table.write = 0;

    terminals[exec_terminal].pcb->open_files[fd].inode_num = 0;
    terminals[exec_terminal].pcb->open_files[fd].file_pos = 0;
    terminals[exec_terminal].pcb->open_files[fd].flags = FLAG_FREE;

}

/*
 * DESCRIPTION: initializes PCB to default values
 *
 * INPUTS: pcb_ptr -- pointer to PCB to be initialized, cmd -- parsed executable command name, argv -- argument
 * arg_num -- number of arguments (1 or 0), pid -- current process id
 * 
 * OUTPUTS: none
 * 
 * SIDE EFFECTS: Initializes PCB
 *
 */ 
void pcb_init(PCB_t* pcb_ptr , uint8_t* cmd, uint8_t (*argv)[MAX_ARGS], uint32_t pid, uint32_t arg_num)
{
    // pcb_ptr = (PCB_t *)(EIGHT_MB_SIZE - (pid + 1) * EIGHT_KB_SIZE);
    int i;
    // Initialize the file descriptor array
    for (i = 0; i < 8;i++) {        //max num of open files in pcb
        pcb_ptr->open_files[i].file_op_table.read  = 0;
        pcb_ptr->open_files[i].file_op_table.write = 0;
        pcb_ptr->open_files[i].file_op_table.open  = 0;
        pcb_ptr->open_files[i].file_op_table.close = 0;
        pcb_ptr->open_files[i].inode_num      = 0;
        pcb_ptr->open_files[i].file_pos         = 0;
        pcb_ptr->open_files[i].flags            = FLAG_FREE;
    }

    //initialize stdin and stdout
    pcb_ptr->open_files[0].file_op_table = stdin_fop;        /* stdin  */
    pcb_ptr->open_files[0].inode_num = 0;
    pcb_ptr->open_files[0].file_pos = 0;
    pcb_ptr->open_files[0].flags = FLAG_BUSY;

    pcb_ptr->open_files[1].file_op_table = stdout_fop;       /* stdout */
    pcb_ptr->open_files[1].inode_num = 0;
    pcb_ptr->open_files[1].file_pos = 0;
    pcb_ptr->open_files[1].flags = FLAG_BUSY;   

    pcb_ptr->pid = pid;
    pcb_ptr->parent_pid = 0;

    pcb_ptr->scheduled = 0; // default zero
    pcb_ptr->is_shell = 0;

    pcb_ptr->parent_esp  = 0;
    pcb_ptr->parent_ebp  = 0;
    pcb_ptr->parent_pcb  =  terminals[exec_terminal].pcb;
    pcb_ptr->tss_esp0    =  tss.esp0;

    pcb_ptr->num_args = arg_num; // silly naming here... oh well

    // copies executable and args
    memcpy(pcb_ptr->cmd, cmd, MAX_CMD_LENGTH);
    for(i = 0; i < MAX_ARGUMENT_NUM; i++)
        memcpy(pcb_ptr->argv[i],argv[i], MAX_ARGS);
}

/*
*
* DESCRIPTION: takes input command from execute syscall and parses it
*
*/
int32_t parse_cmd(const uint8_t* command, uint8_t* parsed_cmd, uint8_t (*argv)[MAX_ARGS])
{
    
    int i, j, cmd_length;
    int start = 0;
    int end = 0;

    if (!command) return -1; // NULL command

    if (command[0] == '\0' || command[0] == ' ') return -1; // invalid command

    // clear buffer
    for (i = 0; i < MAX_CMD_LENGTH; i++) {
        parsed_cmd[i] = '\0';
    }

    for (i = 0; i < MAX_ARGS; i++) {
        for (j = 0; j < MAX_ARGUMENT_NUM; j++) {
            // todo check: cat, grep support multiple args?
                argv[j][i] = '\0';
        }

    }

    // skips spaces at beginning
    while(command[start] == ' ') {
        ++start;
        ++end;
    }

    // populate executable name
    while(command[end] != '\0' && command[end] != ' ' && command[end] != '\n') {
        end++;
    }

    // command name too long?
    if (end - start > MAX_CMD_LENGTH) return -1;

    // memcpy(parsed_cmd, &(command[start]), end - start - 1);

    for (i = 0; i < (end - start); i++) {
        // ignores the 'enter' needed for terminal to read 
        if(command[i+start] != CARRIAGE_RETURN) { 
            parsed_cmd[i] = command[i+start];
        }
    }
    
    while(command[end] == ' ') end++;

    cmd_length = strlen((int8_t*)command);

    // populate args
    for( j = 0; end < cmd_length; j++ )
    {
        // start of this argument is end of last
        start = end;

        // TODO: multiple args?
        if(j >= MAX_ARGUMENT_NUM)
        {
            printf("Too many arguments: maximum number is: %d", MAX_ARGUMENT_NUM);
            return -1;
        }

        // skips leading spaces between args
        while (command[start] == ' ') { 
            start++; 
            end++; 
        }

        // gets arg length 
        while (command[end] != ' '  && command[end] != '\0' && command[end] != '\n') end++;

        // arg length check
        if(end - start > MAX_ARGS) return -1;

        // copies arg to argv
        for (i = 0; i < (end - start); i++) {
            if(command[i+start] != CARRIAGE_RETURN) {
                argv[j][i] = command[i+start];
            }
        }

    }

    // exit check - use -2 because -1 is for errors
    if (strncmp("exit", (int8_t*)parsed_cmd, 4) == 0) return -2;

    return j;
}

/* DESCRIPTION: Calculates PCB given a pid
 *
 * INPUTS: pid -- the pid to obtain the PCB of
 *
 * OUTPUTS: PCB pointer
 */
PCB_t* get_pcb(uint32_t pid){
    return (PCB_t *)(EIGHT_MB_SIZE - (pid + 1) * EIGHT_KB_SIZE);
}

//null fop placeholders
int32_t null_read(int32_t fd, void *buf, int32_t nbytes)
{
    return -1;
}

int32_t null_write(int32_t fd, const void *buf, int32_t nbytes)
{
    return -1;
}

int32_t null_open(const uint8_t *fname)
{
    return -1;
}
int32_t null_close(int32_t fd)
{
    return -1;
}
