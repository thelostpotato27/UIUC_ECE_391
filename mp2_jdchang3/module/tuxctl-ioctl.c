/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)


// void init_TUX_controller(struct tty_struct* tty);
// int set_TUX_buttons(struct tty_struct* tty, unsigned long arg);
// int set_TUX_LED(struct tty_struct* tty, unsigned long arg);
int ack;                                //synchronization variable
unsigned char stored_packets [2];       //packets stored from tuxctl_handle_packet
unsigned char prev_stored_packets [6];  //previous packets in case interrupts for LED

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

    switch(a){
        // Opcode for unlocking LED to TUX
        case MTCP_ACK:
            ack = 0;
            return;

        // Button state is changed (pressed/released) and local storage of buttons must be updated
        case MTCP_BIOC_EVENT:
            stored_packets[0] = b;
            stored_packets[1] = c;
            return;
        
        // Set the state of the TUX to what it was before the reset, which is stored in prev_stored_packets
        case MTCP_RESET:
            init_TUX_controller(tty);
            tuxctl_ldisc_put(tty,prev_stored_packets,6);
            return;
        
        default:
            return;
    }

    /*printk("packet : %x %x %x\n", a, b, c); */
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
    switch (cmd) {
    // initialize the TUX controller
	case TUX_INIT:
        init_TUX_controller(tty);
        return 0;

    // gets the button inputs of the TUX controller somewhere the user can use
	case TUX_BUTTONS:
        set_TUX_buttons(tty, arg);

    // set LED of TUX controller based off of arg input
	case TUX_SET_LED:
        if (ack == 1){
            return 0;
        }
        ack = 1;
        return set_TUX_LED(tty, arg);

	case TUX_LED_ACK:
        return -EINVAL;
	case TUX_LED_REQUEST:
        return -EINVAL;
	case TUX_READ_LED:
        return -EINVAL;
	default:
	    return -EINVAL;
    }
}

//initializes the TUX to allow user to set LED and enable button interupts
//input: struct tty_struct* tty
//output: none
//side effect: initializes TUX controller
void init_TUX_controller(struct tty_struct* tty){
    ack = 1;
    unsigned char tux_init_cmd[2];
    tux_init_cmd[0] = MTCP_BIOC_ON;
    tux_init_cmd[1] = MTCP_LED_USR;

    tuxctl_ldisc_put(tty, tux_init_cmd, 2);
    return;
}


//gets the current state of the button inputs of the TUX, rearrange the format to be suitable
//for input to the user, and return
//input: struct tty_struct* tty, unsigned long arg
//output: 0 if successful, >0 otherwise
//side effects: stores the current TUX button state into the arg memory location specified
int set_TUX_buttons(struct tty_struct* tty, unsigned long arg){
    unsigned char rldu_cbas_holder, temp1, temp2, temp3, temp4;
    temp1 = stored_packets[0] & 0xF;            //store the first 4 bits of byte 1, represent CBAS
    temp2 = (stored_packets[1] & 0xF) << 4;     //store the first 4 bits of byte 2, represents RDLU0000
    temp3 = (temp2 & 0x40)>>1;                  //we need to swap D and L for the final output
    temp4 = (temp2 & 0x20)<<1;
    temp2 = temp2 & 0x90;                       //0x40 masks the 6th bit, 0x20 masks the 5th bit, and 0x90 masks bit
                                                //7 and 4

    rldu_cbas_holder = temp1|temp2|temp3|temp4;
    if (copy_to_user((unsigned long*)arg, &rldu_cbas_holder ,sizeof(rldu_cbas_holder)) != 0){
		return -EINVAL;
	} else {
	    return 0;
	}
}


//sets the LED on TUX based on the input arg
//input: struct tty_struct* tty, unsigned long arg
//output: 0 if successful, >0 otherwise
//side effects: lights up the TUX LED in accordance with the arg input
int set_TUX_LED(struct tty_struct* tty, unsigned long arg){
    //HEX equivalent of #0-15 represented in the TUX input format shown in mtcp.h
    unsigned char hex_to_led[16] ={0xE7, 0x06, 0xCB, 0x8F, 0x2E, 0xAD, 0xED, 0x86, 0xEF, 0xAE, 0xEE, 0x6D, 0xE1, 0x4F, 0xE9, 0xE8};
    unsigned char TUX_output[6];    //1 byte for opcode, 1 for which LED's to activate, 4 for LED data
    unsigned char led_on;
    unsigned char dec_on;
    int i;
    for (i = 0; i < 6; i++){
        TUX_output[i] = 0x0;            //clearing data to ensure no weird stuff happens
    }
    TUX_output[0] = MTCP_LED_SET;
    TUX_output[1] = 0x0F;               //mask to give permission to turn on all LED

    led_on = (arg>>16) & 0x0F;          //getting led_on and dec_on from arg
    dec_on = (arg>>24) & 0x0F;

    for(i = 0; i < 4; i++){
        if ((led_on >> i) & 0x1){
            TUX_output[i + 2] = hex_to_led[(arg>> i*4) & 0xF];
        }
        if ((dec_on >> i) & 0x1){
            TUX_output[i + 2] = TUX_output[i + 2] | 0x10;           //bit location for decimal point
        }
    }
    for (i = 0; i < 6; i++){
        prev_stored_packets[i] = TUX_output[i];
    }
    tuxctl_ldisc_put(tty,TUX_output,6);
    return 0;
}