#include "rtc.h"

// volatile int intr_flag; // interrupt flag

/* Virtualized vars for multiple terminals */
int rtc_on[3] = {0, 0, 0};
volatile int rtc_intr_flag[3] = {0, 0, 0};

// These virtualizations allow the illusion
// of different RTC frequencies by only
// sending a real interrupt once counter
// reaches 0.
int rtc_counter[3] = {0, 0, 0};
int rtc_period[3] = {0, 0, 0};

/*
 * DESCRIPTION: Initializes frequency of rtc and enables periodic interrupts.
 *
 * INPUTS: none
 *
 * OUTPUTS: none
 *
 * SIDE EFFECTS: Initializes RTC registers
 */

void rtc_init(void)
{
   char prev;

   // initialize Register B (see rtc.h) by turning on periodic interrupts
   outb((DISABLE_NMI | REG_B), RTC_IDXPORT); // chose Register B with NMI disabled
   prev = inb(RTC_RWPORT);
   outb((DISABLE_NMI | REG_B), RTC_IDXPORT); // now have to write to this port
   outb((prev | ENABLE_PERIODIC_INTERRUPT), RTC_RWPORT); 

   // enables interrupts
   enable_irq(RTC_IRQ);

   // // initialize frequency by writing to Register A
   // outb((DISABLE_NMI | REG_A), RTC_IDXPORT);
   // prev = inb(RTC_RWPORT);
   // outb(REG_A, RTC_IDXPORT);
   // prev = prev & 0xF0;     // bitmask for bits[7:4]
   // outb((prev | TARGET_FREQ_WRITE), RTC_RWPORT);

   // // initializing rtc handling and frequency
   // intr_flag = 0;
   // rtc_open(NULL);
}

/*
 * DESCRIPTION: handles rtc interrupts
 * 
 * INPUTS: none
 * 
 * OUTPUTS: none
 * 
 * SIDE EFFECTS: prints + to terminal to show frequency
 */
void rtc_handler(void)
{
   int i;
   // cli();

   // print 0 to terminal (for testing)
   // if (print_freq) {
   //    printf("+");
   // }

   // selecting reg C
   outb(REG_C, RTC_IDXPORT);
   inb(RTC_RWPORT);

   for (i = 0; i < 3; i++) { // 3 terminals
      rtc_counter[i]--;

      // sends actual interrupt at virtual frequency
      if (rtc_counter[i] == 0) {
         rtc_intr_flag[i] = 1;
         rtc_counter[i] = rtc_period[i];
      }
   }

   // interrupt flags on
   // intr_flag = 1;
   send_eoi(RTC_IRQ);
   // sti();
}

/*
 * DESCRIPTION: Opens the rtc
 * 
 * INPUTS: file -- filename (not used)
 * 
 * OUTPUTS: returns 0 upon success
 * 
 * SIDE EFFECTS: sets a default rtc frequency of 2 Hz
 */
int32_t rtc_open(const uint8_t* file) {
   // initializing 2 Hz frequency
   int32_t freq = 2;
   // rtc_write(NULL, &freq, 4); // writing 4 bytes

   // rtc is running on executing terminal
   rtc_on[exec_terminal] = 1;

   // how often the virtual RTC will send interrupt
   // rtc_period[exec_terminal] = TARGET_FREQ / freq;

   // rtc_counter[exec_terminal] = rtc_period[exec_terminal];
   rtc_write(0, &freq, 4); // writing 4 bytes

   return 0;
}

/*
 * DESCRIPTION: Closes the rtc
 * 
 * INPUTS: fd -- file descriptor (no used)
 * 
 * OUTPUTS: returns 0 upon success
 * 
 * SIDE EFFECTS: none
 */
int32_t rtc_close(int32_t fd) {

   rtc_on[exec_terminal] = 0;
   return 0;
} 

/*
 * DESCRIPTION: Waits for an RTC interrupt
 * 
 * INPUTS: fd -- file descriptor (not used)
 *         buf -- buffer (not used)
 *         bytes -- size of buffer in bytes (not used)
 * 
 * OUTPUTS: returns 0 upon completion
 * 
 * SIDE EFFECTS: sets a default rtc frequency of 2 Hz
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t bytes) {
   // intr_flag = 0;

   // // wait for interrupt flag
   // while(intr_flag == 0) {}

   // // interrupt flag was set to 1!

   if (rtc_on[exec_terminal] == 1) {
      rtc_intr_flag[exec_terminal] = 0;
   }

   sti();

   // wait for interrupt flag
   while(rtc_intr_flag[exec_terminal] == 0) {}
   
   cli();

   return 0;
}

/*
 * DESCRIPTION: Writes a new frequency to the rtc
 * 
 * INPUTS: fd -- file descriptor (not used)
 *         buf -- buffer with new rtc frequency
 *         bytes -- size of buffer in bytes (if it is not 4 return -1)
 * 
 * OUTPUTS: returns 0 upon success and -1 upon failure
 * 
 * SIDE EFFECTS: changes rtc frequency to frequency in buffer
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t bytes) {

   // locals
   int32_t* freq_pt;
   int32_t freq;
   // int32_t freq_logged;

   if (buf == NULL || bytes != 4) { // param check
      return -1;
   }

   // getting frequency
   freq_pt = (int32_t *)buf;
   freq = *freq_pt;

    // frequency param check
    if (freq < 2 || freq > MAX_FREQ)
      return -1;

   // only takes powers of 2
   if ((freq & (freq - 1)) != 0) return -1;

    rtc_counter[exec_terminal] = TARGET_FREQ / freq;

    rtc_period[exec_terminal] = rtc_counter[exec_terminal];

   // int32_t* freq_pt;
   // int32_t freq;
   // int freq_logged, freq_mathed;
   // char regA, comb;
   
   // // cli :p
   // // cli();

   // // getting frequency
   // freq_pt = (int32_t *)buf;
   // freq = *freq_pt;

   // // bounds checking for the frequency (must be between 2 and 1024)
   // if (freq < 2) {
   //    // sti();
   //    return -1;
   // }
   // if (freq > TARGET_FREQ) {
   //    // sti();
   //    return -1;
   // }

   // // rtc rate but log_2 (backwards exponent)
   // freq_logged = 0;

   // // only takes powers of 2
   // if ((freq & (freq - 1)) != 0) return -1;

   // while (freq > 1) {

   //    // increment the log
   //    freq_logged++;

   //    freq /= 2;
   // }

   // // rate must be inverted and less than 16
   // freq_mathed = 16 - freq_logged;

   // // make sure it is less than 16 (4 lsb)
   // freq_mathed = freq_mathed & 0x0F;

   // // setting to register A
   // outb((DISABLE_NMI | REG_A), RTC_IDXPORT);
   // regA = inb(RTC_RWPORT);
   // outb((DISABLE_NMI | REG_A), RTC_IDXPORT);

   // // combine prev rate and new rate (take regA[7:4])
   // comb = (regA & 0x0F0) | freq_mathed;
   // outb(comb, RTC_RWPORT);

   // intr_flag = 0;
   // sti();

   // success
   return 4; // always 4 bytes written
}
