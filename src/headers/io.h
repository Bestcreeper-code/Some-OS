#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>


#include "string.h"

#define KEY_ESCAPE     27
#define KEY_BACKSPACE  8
#define KEY_ENTER      '\n'
#define KEY_UP         0x80
#define KEY_DOWN       0x81
#define KEY_LEFT       0x82
#define KEY_RIGHT      0x83
#define KEY_HOME       0x84

extern volatile uint16_t* video_memory;
extern int vgaX, vgaY;

// Keyboard state
extern bool shift_pressed, caps_lock_on, ctrl_pressed, alt_pressed, altgr_pressed;
extern char current_Language;
// VGA/Screen output
void put_char(int x, int y, uint8_t c, uint8_t color);
char get_char(int x, int y);
void Scroll_Down(void);
void ClearScreen(void);
void move_cursor(int x, int y);
void enable_cursor(uint8_t start, uint8_t end);

// Text Output
int printstr(const char* str);
int printlen(const char* buffer, unsigned int length);
int print_hex32(uint32_t byte);
int print_number(int num);
int print_unsigned_number(uint32_t num);
int printf(const char* format,...);
int printLine(const char* str, int line);

// Text Color
// set current print color
void set_print_color(char color);

// Input
unsigned char GetInputChar(void);
unsigned char GetInputCharNonBlocking(void);
#endif
