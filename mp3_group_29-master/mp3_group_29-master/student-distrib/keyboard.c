#include "keyboard.h"
#include "idt.h"
#include "types.h"

#define KEYBOARD_IRQ 1
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_COMMAND_PORT 0x64

#define BUFFER_SIZE 128

/*
 * DESCRIPTION: Initializes Keyboard.
 *
 * INPUTS: None
 *
 * OUTPUTS: None
 *
 * SIDE EFFECTS: Enables the keyboard irq
 */
void KB_init()
{
    enable_irq(KEYBOARD_IRQ);
}

static const uint8_t scanCodes[128] = {
        0,
        27,
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
        '0',
        '-',
        '=',
        '\b',
        '\t',
        'q',
        'w',
        'e',
        'r',
        't',
        'y',
        'u',
        'i',
        'o',
        'p',
        '[',
        ']',
        '\n',
        0,
        'a',
        's',
        'd',
        'f',
        'g',
        'h',
        'j',
        'k',
        'l',
        ';',
        '\'',
        '`',
        0,
        '|',
        'z',
        'x',
        'c',
        'v',
        'b',
        'n',
        'm',
        ',',
        '.',
        '/',
        0,
        '*',
        0,
        ' ',
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        '-',
        0,
        0,
        0,
        '+',
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
};

static const uint8_t shiftScanCodes[128] = {
    0,
    27,
    '!',
    '@',
    '#',
    '$',
    '%',
    '^',
    '&',
    '*',
    '(',
    ')',
    '_',
    '+',
    '\b',
    '\t',
    'Q',
    'W',
    'E',
    'R',
    'T',
    'Y',
    'U',
    'I',
    'O',
    'P',
    '{',
    '}',
    '\n',
    0,
    'A',
    'S',
    'D',
    'F',
    'G',
    'H',
    'J',
    'K',
    'L',
    ':',
    '\"',
    '~',
    0,
    '\\',
    'Z',
    'X',
    'C',
    'V',
    'B',
    'N',
    'M',
    '<',
    '>',
    '?',
    0,
    '*',
    0,
    ' ',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    '-',
    0,
    0,
    0,
    '+',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

// Flags for modifier keys
uint8_t shift_pressed = 0;
uint8_t ctrl_pressed = 0; // can be greater than one because of left and right shift
uint8_t caps_pressed = 0;
uint8_t alt_pressed = 0;

/*
 * DESCRIPTION: Initializes Keyboard.
 *
 * INPUTS: None
 *
 * OUTPUTS: None
 *
 * SIDE EFFECTS: Handles keyboard interrupts and prints to screen
 */
void keyboard_handler()
{
    uint8_t data = inb(KEYBOARD_DATA_PORT);

    switch (data)
    {
    // check modifiers
    case 0x2A: //shift pressed
    case 0x36:
        shift_pressed++;
        send_eoi(KEYBOARD_IRQ);
        return;
    case 0xAA: //shift released
    case 0xB6:
        shift_pressed--;
        send_eoi(KEYBOARD_IRQ);
        return;
    case 0x1D: //control pressed
        ctrl_pressed++;
        send_eoi(KEYBOARD_IRQ);
        return;
    case 0x9D: //control released
        ctrl_pressed--;
        send_eoi(KEYBOARD_IRQ);
        return;
    case 0x3A: //caps lock toggled
        caps_pressed = !caps_pressed;
        send_eoi(KEYBOARD_IRQ);
        return;
    case 0xBA:
        send_eoi(KEYBOARD_IRQ);
        return;
    case 0x38: //alt pressed
        alt_pressed++;
        send_eoi(KEYBOARD_IRQ);
        return;
    case 0xB8: //alt released
        alt_pressed--;
        send_eoi(KEYBOARD_IRQ);
        return;
    default:
        break;
    }

    if(alt_pressed > 0 && shift_pressed == 0)
    {
        switch(data)
        {
            case 0x3B: //F1
                switch_terminal(0);
                restore_vidmem();
                set_vidmem(exec_terminal);
                send_eoi(KEYBOARD_IRQ);
                return;
                // break;

            case 0x3C: //F2
                switch_terminal(1);
                restore_vidmem();
                set_vidmem(exec_terminal);
                send_eoi(KEYBOARD_IRQ);
                return;
                // break;

            case 0x3D: //F3
                switch_terminal(2);
                restore_vidmem();
                set_vidmem(exec_terminal);
                send_eoi(KEYBOARD_IRQ);
                return;
                // break;

            default:
                send_eoi(KEYBOARD_IRQ);
                return;
        }
    }

    restore_vidmem();

    if (data >= 128) // dont crash on non printable chars
    {
        set_vidmem(exec_terminal);
        send_eoi(KEYBOARD_IRQ);
        return;
    }

    //handle uppercase chars
    if (((shift_pressed > 0 && caps_pressed == 0) ||
         (shift_pressed == 0 && caps_pressed > 0)) &&
        data < 128)
    {
        if(scanCodes[data] >= 'a' && scanCodes[data] <= 'z')
        {
            data = shiftScanCodes[data];            
        }
        else
        {
            if(caps_pressed > 0)
            {
                data = scanCodes[data];
            }
            else
            {
                data = shiftScanCodes[data];
            }
        }
    }
    else
    {
        data = scanCodes[data];
    }

    // restore_vidmem();

    //handle newline
    if (data == '\n')
    {
        putc(data, disp_terminal);
        buffer_char(data);
        set_vidmem(exec_terminal);
        send_eoi(KEYBOARD_IRQ);
        return;
    }
    else if(data == '\b')
    {
        if(terminals[disp_terminal].buffer_length > 0)
        {
            putc(data, disp_terminal);
            // buffer_char(data);
        }
        backspace();
        set_vidmem(exec_terminal);
        send_eoi(KEYBOARD_IRQ);
        return;
    }

    if (ctrl_pressed > 0 && data == 'l')
    {
        clear();
        reset_cursor();

        if(terminals[disp_terminal].pcb->is_shell)
           printf("391OS> ");

        set_vidmem(exec_terminal);
        send_eoi(KEYBOARD_IRQ);
        return;
    }

    if(data == 0) // don't crash on unused char
        // set_vidmem(exec_terminal);
        // send_eoi(KEYBOARD_IRQ);
        return;

    putc(data, disp_terminal);
    buffer_char(data);

    set_vidmem(exec_terminal);
    send_eoi(KEYBOARD_IRQ);
}
