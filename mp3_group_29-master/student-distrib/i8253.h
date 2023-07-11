#include "lib.h"
#include "types.h"
#include "keyboard.h"
#include "paging.h"

#define FREQUENCY 			    11932
#define CHANNEL_0 				0x40
#define CMD_REG					0x43
#define PIT_PORT                0x34
#define PIT_IRQ                 0x00

void i8253_init(void);
void pit_handler(void);

