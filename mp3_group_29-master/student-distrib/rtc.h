/*
 * Provides RTC functionality, including initialization
 * and interrupt support.
 *
 */
#ifndef RTC_H
#define RTC_H

#include "i8259.h"
#include "lib.h"
#include "keyboard.h"
#include "types.h"

/* See https://wiki.osdev.org/RTC for reference and initialization code. */

/* Additional references for Registers: https://stanislavs.org/helppc/cmos_ram.html.*/
#define RTC_IDXPORT 0x70 // Specifies index/"register number", disables NMI
#define RTC_RWPORT 0x71 // read/write from byte of CMOS configuration spaces
#define RTC_IRQ 0x8 // IRQ8 on PIC

/* Maximum and target frequency as specified in MP3 writeup. */
#define MAX_FREQ 8192
#define TARGET_FREQ 1024
/* This sets frequency in Register A to 1024 Hz -- see links for details. */
#define TARGET_FREQ_WRITE 0x06

/* Example Usage:
 *
 * Register B with NMI enabled: REG_B
 *
 * Register B with NMI disabled: DISABLE_NMI | REG_B
 */


#define REG_A 0xA // Registers A-C
#define REG_B 0xB
#define REG_C 0xC
#define DISABLE_NMI 0x80 // disables NMI for registers A-C

#define ENABLE_PERIODIC_INTERRUPT 0x40

/* Global variables. */

// DELETE THESE NOTES WHEN YOU ARE DONE
// You probably want variables for:
// - interrupt flag
// - counter
// - variables to help keep track of active process, current frequency, etc.
// MAKE SURE TO MODIFY rtc_init() if these variables need to be initialized

/* Publicly available functions. */

/* Initializes RTC. */
void rtc_init(void);

/* Called when RTC interrupt occurs. */
void rtc_handler(void);

/* Opens RTC. */
int32_t rtc_open(const uint8_t* file);

/* Closes RTC.*/
int32_t rtc_close(int32_t fd); // fd is file descriptor

/* Waits for RTC interrupt. */
int32_t rtc_read(int32_t fd, void* buf, int32_t bytes);

/* Writes a new frequency. */
int32_t rtc_write(int32_t fd, const void* buf, int32_t bytes);

/* only enable when testing pls*/
// volatile int print_freq; // set it to print things to see freq

/* Helper functions. */

// DELETE THESE NOTES WHEN YOU ARE DONE
// You probably want a frequency-setting helper. It should
// check to see if the new frequency is valid and update
// appropriate variables.

#endif
