#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "lib.h"
#include "i8259.h"
#include "syscalls.h"
#include "terminal.h"

// keyboard modifier flags
uint8_t caps_flag;
uint8_t shift_flag;
uint8_t alt_flag;
uint8_t ctrl_flag;
volatile uint8_t enter_pressed[3];

// keyboard functions
extern void KB_init(void);
extern void keyboard_handler(void); // interrupt handler

#endif
