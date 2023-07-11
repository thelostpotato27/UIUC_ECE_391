#include "terminal.h"

// static char buffer[128];
// volatile static int enter_pressed = 0;
// int buffer_length = 0;

// private function helpers

/*
 *  DESCRIPTION: Clears line buffer of all terminals
 *
 *  INPUT: none
 *
 *  OUTPUT: none
 *
 *  SIDE EFFECTS: resets all terminal buffers
 */
void clear_buffer(void)
{
    int i;
    // fill buffer with NULL
    for(i = 0; i < 128; i++)
         terminals[disp_terminal].buffer[i] = NULL;

     terminals[disp_terminal].buffer_length = 0;
}

/*
 *  DESCRIPTION: Backspaces, fills buffer with space char
 *
 *  INPUT: none
 *
 *  OUTPUT: none
 *
 *  SIDE EFFECTS: modifies buffer of displayed terminal
 */
void backspace(void) {
    if (terminals[disp_terminal].buffer_length > 0) terminals[disp_terminal].buffer_length--;

    // changes buffer so it doesn't print past end
    terminals[disp_terminal].buffer[terminals[disp_terminal].buffer_length] = 0x10; 
}

//Wrapper functions, look down for full implementation
int32_t terminal_open(const uint8_t* filename)
{
    return private_terminal_open();
}

int32_t terminal_close(int32_t fd)
{
    return private_terminal_close();
}

int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
{
    return private_terminal_read((char*)buf, nbytes);
}

int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes)
{
    return private_terminal_write((char*)buf, nbytes);
}

/*
 * DESCRIPTION: Initializes terminal variables.
 *
 * INPUTS: None
 * 
 * OUTPUTS: none
 * 
 * SIDE EFFECTS: sets terminal variables
 */
void init_terminal() {
    clear();
    reset_cursor();
    clear_buffer(); 

    
    int i;
	for (i = 0; i < 3; i++){
         terminals[i].pcb = (PCB_t*)NULL;
         terminals[i].x = 0;
         terminals[i].y = 0;
         terminals[i].buffer_length = 0;
         terminals[i].num_programs = 0;
         terminals[i].vid_mem = VIDEO_ADDRESS + (i + 1) * FOUR_KB_SIZE; // offset per terminal
	}
}

/*
 * DESCRIPTION: switches terminal to the terminal ID. Moves
 * current terminal's memory into vidmem of terminal and
 * loads new terminal's video memory into the displayed
 * memory.
 */
void switch_terminal(int32_t terminal_num) {

    int32_t old_terminal = disp_terminal;

    // switch to same terminal
    if (old_terminal == terminal_num) return;


    if (terminal_num < 0 || terminal_num > 2 || num_programs > 6)  {// 6 programs total
        return;
    }

    // update new display terminal
    disp_terminal = terminal_num;

    restore_vidmem();

    // saves old terminal information into memory
    memcpy((void *)terminals[old_terminal].vid_mem, (const void*)VIDEO_ADDRESS, FOUR_KB_SIZE);

    // displays new terminal
    memcpy((void *)VIDEO_ADDRESS, (const void *)terminals[disp_terminal].vid_mem,  FOUR_KB_SIZE);

    draw_cursor();
}

/*
 * DESCRIPTION: Opens terminal. 
 *
 * INPUTS: none
 * 
 * OUTPUTS: none
 * 
 * SIDE EFFECTS: clears screen and buffer
 */
int private_terminal_open()
{
    // enter_pressed = 0;
    // buffer_length = 0;

    clear();
    reset_cursor();
    clear_buffer();
    return 0;
}

/*
 * DESCRIPTION: Closes terminal. Just returns.
 *
 * INPUTS: none
 * 
 * OUTPUTS: none
 * 
 * SIDE EFFECTS: none
 */
int private_terminal_close()
{
    clear_buffer();
    return 0;
}

/*
 * DESCRIPTION: Reads user input from terminal and stores it into a buffer. Limited to 128.
 *
 * INPUTS: data -- buffer to be written to, size -- # of chars to be read
 * 
 * OUTPUTS: number of bytes read
 * 
 * SIDE EFFECTS: modifies internal buffer with user input
 */

int private_terminal_read(char *data, int size)
{
    // buffer check
    if (data == NULL) return -1;

    int i;
    int bytes_read = 0;

    // clear buffer at executing terminal only
     terminals[exec_terminal].buffer_length = 0;

    // clear buffers
    for(i = 0; i < MAX_BUF_SIZE; i++) // 128 is maximum buffer size
    {
        data[i] = ' ';
        terminals[exec_terminal].buffer[i] = ' ';
    }

    sti(); // waits for user input

    while(terminals[exec_terminal].buffer_length < MAX_BUF_SIZE - 1 && !enter_pressed[exec_terminal]);

    cli();

    // extra +1 for newline with terminal write
    bytes_read = terminals[exec_terminal].buffer_length + 1;

    for (i = 0; i < bytes_read; i++) {
        data[i] = terminals[exec_terminal].buffer[i];
    }

    // newline
    data[i] = '\n';

    enter_pressed[exec_terminal] = 0; // clears enter press and buffer
    terminals[exec_terminal].buffer_length = 0;

    // clear buffers
    for(i = 0; i < MAX_BUF_SIZE; i++) // 128 is maximum buffer size
    {
        terminals[exec_terminal].buffer[i] = ' ';
    }
    
    // sti();

    return bytes_read;
}

/*
 * DESCRIPTION: Writes data from buffer to terminal.
 *
 * INPUTS: data -- text to be written, size -- # of chars to write
 * 
 * OUTPUTS: # of characters to write
 * 
 * SIDE EFFECTS: none
 */

int private_terminal_write(char *data, int size)
{
    if (data == NULL) return -1; // invalid buffer

    int i = 0;
    for (i = 0; i < size; i++) // echoes input to screen
    {
        putc(data[i], exec_terminal); 
    }
    return size;
}

/*
 * DESCRIPTION: Populates buffer; handles enter and backspace.
 *
 * INPUTS: data -- either character or newline/backspace command
 * 
 * OUTPUTS: none
 * 
 * SIDE EFFECTS: modifies buffer
 */

void buffer_char(char data)
{
    switch (data)
    {
    case '\n': // puts newline in last position
        enter_pressed[disp_terminal] = 1;
        if (terminals[disp_terminal].buffer_length >= 128)
        {
            terminals[disp_terminal].buffer[128 - 1] = '\n';
        }
        else
        {
            terminals[disp_terminal].buffer[terminals[disp_terminal].buffer_length] = '\n';
        }
        break;
    case '\b': // backspace
        if (terminals[disp_terminal].buffer_length > 0)
        {
            if (terminals[disp_terminal].buffer_length < 128)
            {
                terminals[disp_terminal].buffer[terminals[disp_terminal].buffer_length - 1] = ' ';
            }
            terminals[disp_terminal].buffer_length--;
        }
        break;
    default: // just populates buffer with character
        if (terminals[disp_terminal].buffer_length < 128 - 1)
        {
            terminals[disp_terminal].buffer[terminals[disp_terminal].buffer_length] = data;
            terminals[disp_terminal].buffer_length++;
        }
        else
        {
            terminals[disp_terminal].buffer_length++;
        }
        break;
    }
}

/*
 * DESCRIPTION: Draws cursor based on display terminal's coordinates.
 * Code from https://wiki.osdev.org/Text_Mode_Cursor. 
 *
 * INPUTS: none
 * 
 * OUTPUTS: none
 * 
 * SIDE EFFECTS: Draws disp_terminal's cursor to screen
 *
 */

void draw_cursor(void) {
    
    uint16_t pos =  terminals[disp_terminal].y * SCREEN_COLS +  terminals[disp_terminal].x;

    outb(0x0F, 0x3D4);
    outb((uint8_t)(pos & 0xFF), 0x3D5);
    outb(0x0E, 0x3D4);
    outb((uint8_t)((pos >> 8) & 0xFF), 0x3D5);   
}

/*
 * DESCRIPTION: Reset cursor's position. 
 *
 * INPUTS: none
 * 
 * OUTPUTS: none
 * 
 * SIDE EFFECTS: Resets x,y, of display terminal to 0.
 *
 */

void reset_cursor(void) {
    terminals[disp_terminal].x = 0;
    terminals[disp_terminal].y = 0;
    draw_cursor();
}
