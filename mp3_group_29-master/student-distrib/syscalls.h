/*
 * syscalls.h - Contains syscalls for system operations.
 * vim:ts=4 noexpandtab
 */

#ifndef SYSCALLS_H
#define SYSCALLS_H

#ifndef ASM

#include "x86_desc.h"
#include "lib.h"
#include "types.h"
#include "filesys.h"
#include "terminal.h"
#include "rtc.h"
#include "paging.h"
#include "syscall_help.h"

// Assembly linkage for syscalls
void syscall_wrap(void);
// Assembly linkage for context switch
void move_args(uint32_t parent_ebp, uint32_t parent_esp, uint8_t global_status);

// Syscalls
int32_t halt(uint8_t status);
int32_t execute(const uint8_t *command);
int32_t read(int32_t fd, void *buf, int32_t nbytes);
int32_t write(int32_t fd, const void *buf, int32_t nbytes);
int32_t open(const uint8_t *filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t *buf, int32_t nbytes);
int32_t vidmap(uint8_t **screen_start);
// Extra Credit -- returns failure (-1) for now
int32_t set_handler(int32_t signum, void *handler_address);
int32_t sigreturn(void);

void context_switch(uint32_t entry);
// helpers
// void clearFd(PCB_entry_t* openFd);
// int32_t clearProcess(int32_t pid);

// void fop_init();

#endif // ASM
#endif // SYSCALLS_H
