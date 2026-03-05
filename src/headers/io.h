#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>



#include "kernel_data.h"
#include "multiboot_info.h"
//1024x768
#define K_TERMINAL_WIDTH  (Multiboot_info->framebuffer_width /8)
#define K_TERMINAL_HEIGHT (Multiboot_info->framebuffer_height /16)

#define KEY_ESCAPE     27
#define KEY_BACKSPACE  8
#define KEY_ENTER      '\n'
#define KEY_UP         0x80
#define KEY_DOWN       0x81
#define KEY_LEFT       0x82
#define KEY_RIGHT      0x83
#define KEY_HOME       0x84
#define CTRL_KEY_COMBO 159

#define SET_KEYBOARD_MOD_FLAG(flag, state) \
    (KEYBOARD_MOD_KEYS_FLAGS) = (state) ? (KEYBOARD_MOD_KEYS_FLAGS) | (flag) : ((KEYBOARD_MOD_KEYS_FLAGS) & ~(flag))

#define GET_KEYBOARD_MOD_FLAG(flag) \
    (( KEYBOARD_MOD_KEYS_FLAGS) & (flag))

#define ControlCombo(key) CTRL_KEY_COMBO + key - 'A'

#define ALT_PRESSED 0x1
#define ALTGR_PRESSED 0x2
#define CTRL_PRESSED 0x4  
#define SHIFT_PRESSED 0x08
#define CAPSLOCK_ON 0x10

extern volatile uint16_t* video_memory;
extern int vgaX, vgaY;

// Keyboard state
extern char current_Language;
void init_keyboard();
// VGA/Screen output
void put_char(int x, int y, uint8_t c, uint8_t color);

void Scroll_Down();
void ClearScreen();
void move_cursor(int x, int y);


// Text Output
int printstr(const char* str);
int printlen(const char* buffer, unsigned int length);
int print_hex32(uint32_t byte);
int print_number(int num);
int print_unsigned_number(uint32_t num);
int printf(const char* format,...) __attribute__ ((format (printf, 1, 2)));
int printLine(const char* str, int line);

// Text Color
// set current print color
void set_print_color(char color);

// Input
// unsigned char GetInputChar();
unsigned char GetInputCharNonBlocking();

unsigned char getc();
unsigned char getc_nb();

void reset_input_buffer();

void get_string(char* buffer);
void get_string_after_index(int start, char* buffer);


//sprintf & others


int sprintf(char* buffer, const char* format, ...)    __attribute__ ((format (printf, 1, 3)));
int snprintf(char* buffer, int size, const char* format, ...) __attribute__ ((format (printf, 1, 4)));
int vsprintf(char* buffer, const char* format, va_list args);
int vsnprintf(char* buffer, int size, const char* format, va_list args);

int write_char(char* buffer, int pos, char c, int size);
int write_str(char* buffer, int pos, const char* s, int size);
int write_number(char* buffer, int pos, int num, int size);
int write_uint32(char* buffer, int pos, uint32_t num, int size);
int write_hex32(char* buffer, int pos, uint32_t num, int size);
int write_number_fixed_width(char* buffer, int pos, int num, int width, int size);



unsigned int vga_to_32bit_color(unsigned char fg_vga_color);
#endif