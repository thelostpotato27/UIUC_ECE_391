#ifndef _FILESYS_H
#define _FILESYS_H

#include "lib.h"
#include "types.h"
#include "syscalls.h"

/* Variables. */

inode_t* inode_start;
dentry_t data_entry;
datablock_t* data_block_start;
boot_block_t* boot_block;

/* Public functions. */

/*Initializes file system. */
void filesys_init(uint32_t start_addr);

/* Opens file. */
int32_t file_open(const uint8_t* file);

/* Closes file.*/
int32_t file_close(int32_t fd); // fd is file descriptor

/* Reads 'bytes' bytes from 'buf.' */
int32_t file_read(int32_t fd, void* buf, int32_t bytes);

/* Reads 'bytes' bytes from 'buf.' */
int32_t data_read(int32_t fd, void* buf, uint32_t bytes, uint32_t offset);

/* Should just return (see MP3 writup Checkpoint 2). */
int32_t file_write(int32_t fd, const void* buf, int32_t bytes);

/* Opens directory. */
int32_t dir_open(const uint8_t* file);

/* Should just return (see MP3 writup Checkpoint 2). */
int32_t dir_close(int32_t fd); // fd is file descriptor

/* Reads 'bytes' bytes to 'buf.' */
int32_t dir_read(int32_t fd, void* buf, int32_t bytes);

/* Should just return (see MP3 writup Checkpoint 2). */
int32_t dir_write(int32_t fd, const void* buf, int32_t bytes);

//read directory entry with fname and put it in dentry
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
//read directory entry at index and put it in dentry
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
//read data of length at inode starting at offset and write it into buf
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

extern int32_t get_file_size(int32_t inode_index);
#endif
