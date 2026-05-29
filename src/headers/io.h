#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>



#include "kernel_data.h"
#include "bootloader.h"
//1024x768
#define K_TERMINAL_WIDTH  (get_bootloader_fb_info()->width /8)
#define K_TERMINAL_HEIGHT (get_bootloader_fb_info()->height /16)

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




unsigned int vga_to_32bit_color(unsigned char fg_vga_color);
#endif