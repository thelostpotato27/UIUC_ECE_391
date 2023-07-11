#ifndef TERMINAL_H
#define TERMINAL_H

#include "i8259.h"
#include "lib.h"
#include "keyboard.h"
#include "types.h"
#include "paging.h"

int private_terminal_open();
int32_t terminal_open(const uint8_t* filename);

int private_terminal_close();
int32_t terminal_close(int32_t fd);

int private_terminal_read(char* buffer, int size);
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

int private_terminal_write(char* buffer, int size);
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

void switch_terminal(int32_t terminal_num);
void init_terminal(void);

// helpers
void buffer_char(char data);
void draw_cursor(void);
void reset_cursor(void);
void backspace(void);


void copy_terminal_data(int32_t terminal_num, int32_t cmd);
void clear_terminal_video(void);

#endif
