#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"

uint32_t page_table[table_entries]  __attribute__((aligned (FOUR_KB_SIZE)));

uint32_t page_directory[table_entries]  __attribute__((aligned (FOUR_KB_SIZE)));

uint32_t vid_table[table_entries] __attribute__((aligned(FOUR_KB_SIZE)));


extern void page_directory_init();

void setPD(uint32_t *);
void enablePaging(void);

// user program support
extern void switch_pd(uint32_t addr);

// multiterminal support
extern void switch_vid();
extern void set_vidmem(int32_t terminal_num);
extern void restore_vidmem(void);

extern void remap_program(uint32_t pid);

extern void flushTlb();

#endif
