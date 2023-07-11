/* types.h - Defines to use the familiar explicitly-sized types in this
 * OS (uint32_t, int8_t, etc.).  This is necessary because we don't want
 * to include <stdint.h> when building this OS
 * vim:ts=4 noexpandtab
 */

#ifndef _TYPES_H
#define _TYPES_H

#define NULL 0

#ifndef ASM

/* ------ File System Constants -------- */
/* Useful Constants. Refer to Appendices A & B of MP3 Writeup for more information. */

#define ENTRY_SIZE 64       //size of each direc entry
#define BLOCK_SIZE 4096     //4 KB
#define MAX_NAME_LENGTH 32  //max Byte length

// File Types
#define RTC_ACCESS 0
#define DIRECTORY_ACCESS 1
#define FILE_ACCESS 2


/*-------- Syscall Constants ------------- */

#define SYS_HALT 1
#define SYS_EXECUTE 2
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_GETARGS 7
#define SYS_VIDMAP 8
#define SYS_SET_HANDLER 9
#define SYS_SIGRETURN 10

#define FLAG_FREE 0
#define FLAG_BUSY 1

#define BASE_ADDR 0x800000
#define PROG_OFFSET 0x400000

#define FOUR_MB_SIZE 0x400000
#define EIGHT_MB_SIZE 0x800000
#define EIGHT_KB_SIZE 0x2000
#define FOUR_KB_SIZE 0x1000

// See MP3 Syscalls for values
#define USER_PROGRAM_ADDR 0x08048000
#define USER_PROGRAM_OFFSET 0x00048000

// argument constants
#define MAX_ARGS 40
#define MAX_CMD_LENGTH 10
#define MAX_ARGUMENT_NUM 1

/* ------- Keyboard/Terminal Constants ----------- */
#define SCREEN_COLS 80
#define MAX_BUF_SIZE 128
#define KEYBOARD_IRQ 1

/* ----- paging constants ---- */
#define table_entries   1024
#define table_size      1024 * 4
#define START_ADDRESS       0x000000            //rand num for now as placeholder
#define VIDEO_ADDRESS       0xB8000             //same as in lib.c
#define VIDEO_INDEX         0xB8                //virtual mem location same as physical
#define PAGE_4MB            0x400000            //temp address for 4mb page
#define SIZE          0x80                //set bit 7 to 1, indicates 4MB enabled
#define ENABLE_PG           0x80000001          //set bit 31 to 1, enables paging
#define USER_MEM            0x8000000
#define USER_PAGE           32
#define VIDEO_START 0x08000000
#define VIDEO_END   0x08400000
#define READ_WRITE  0x2
#define USER    0x4
#define PRESENT 0x1

/* ------------------ RTC ------------- */

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

/* Types defined here just like in <stdint.h> */
typedef int int32_t;
typedef unsigned int uint32_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef char int8_t;
typedef unsigned char uint8_t;

/* ---------------- File System ------------------------- */

/* Structs. */
typedef struct dentry_struct{
    int8_t filename[MAX_NAME_LENGTH];
    uint32_t filetype;
    uint32_t inode_num;
    uint8_t reserved[24];       //24B reserved for dentry, as defined in the boot block structure 
}dentry_t;

typedef struct boot_block_struct {
    uint32_t dir_entries;
    uint32_t inode_nums;
    uint32_t data_block_num;
    uint8_t reserved[52];       //52B reserved for boot block, as defined in the boot block structure
    /*above represents the boot sub-block, 64B*/
    /*below represents the 63 other 64B sub-blocks that make up a single data block*/
    dentry_t d[63];
}boot_block_t;

typedef struct inode_struct{
    uint32_t data_length;
    uint32_t data_block[BLOCK_SIZE / 4 - 1]; // each data block is 4B
}inode_t;

typedef struct datablock_struct{
    uint8_t data[BLOCK_SIZE];
} datablock_t;



/*--------- Syscall Structures -------------- */

typedef struct fop_t
{
    int32_t (*open)(const uint8_t *file);
    int32_t (*close)(int32_t file);
    int32_t (*read)(int32_t file, void *buf, int32_t bytes);
    int32_t (*write)(int32_t file, const void *buf, int32_t bytes);
} fop_t;

typedef struct PCB_entry_struct
{
    fop_t file_op_table;
    uint32_t inode_num;
    uint32_t file_pos;
    uint32_t flags;
} PCB_entry_t;

// PCB structure
typedef struct PCB_struct
{
    PCB_entry_t open_files[8]; // max amount of open files in PCB

    struct PCB_struct* parent_pcb;
    uint32_t parent_pid;
    uint32_t parent_esp;
    uint32_t parent_ebp;

    uint32_t pid;
    uint32_t user_esp;
    uint32_t user_ebp;

    uint32_t tss_esp0;

    int8_t argv[MAX_ARGUMENT_NUM][MAX_ARGS];
    int8_t cmd[10];
    uint32_t num_args;

    uint8_t is_shell;
    uint8_t scheduled;

} PCB_t;

/*---------------------------- Terminal Structures ----------------------------*/

/* For multiterminal support */
typedef struct terminal {
         PCB_t*     pcb;
         int32_t    x; // where cursor is located
         int32_t    y; 

         int32_t    vid_mem;
         int32_t    num_programs;

volatile uint8_t    buffer[128]; // max buffer length of 128
volatile int32_t    buffer_length;

} terminal_t;

/* --------- Global Variables ----------- */

fop_t null_fop; // when initializing file descriptor array
fop_t stdin_fop;// = {terminal_open, terminal_close, terminal_read, null_write};
fop_t stdout_fop;// = {terminal_open, terminal_close, null_read, terminal_write};
fop_t dir_fop;// = {dir_open, dir_close, dir_read, dir_write};
fop_t filesys_fop;// = {dir_open, dir_close, dir_read, dir_write};
fop_t rtc_fop;// = {rtc_open, rtc_close, rtc_read, rtc_write};

PCB_t *curr_pcb;
uint32_t cur_pid;
uint32_t parent_pid;
// process at pid
int process_flag[6];
uint8_t global_status;
uint8_t num_programs;

terminal_t  terminals[3];     /* array of all terminals */
int32_t     disp_terminal;           /* The terminal the user sees. */
int32_t     exec_terminal;           /* The terminal currectly executing a program. */

#endif /* ASM */

#endif /* _TYPES_H */
