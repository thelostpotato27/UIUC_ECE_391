#include "lib.h"
#include "types.h"
#include "x86_desc.h"

#include "rtc.h"
#include "keyboard.h"
#include "terminal.h"
#include "filesys.h"

#define CARRIAGE_RETURN 0x0D

extern void init_vars(void); // initializes relevant global variables

extern int32_t valid_fd(int32_t fd);
extern void clear_fd(int32_t fd);

void pcb_init(PCB_t * pcb_ptr, uint8_t* cmd, uint8_t (*argv)[MAX_ARGS], uint32_t arg_num, uint32_t pid);

int32_t parse_cmd(const uint8_t* command, uint8_t* parsed_cmd, uint8_t (*argv)[MAX_ARGS]);
PCB_t* get_pcb(uint32_t pid);

int32_t null_read(int32_t fd, void *buf, int32_t nbytes);
int32_t null_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t null_open(const uint8_t *fname);
int32_t null_close(int32_t fd);
