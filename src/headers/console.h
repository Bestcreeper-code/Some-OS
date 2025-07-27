#ifndef CONSOLE_H
#define CONSOLE_H


#define KEY_ESCAPE     27
#define KEY_BACKSPACE  8
#define KEY_UP         0x80
#define KEY_DOWN       0x81
#define KEY_LEFT       0x82
#define KEY_RIGHT      0x83
#define KEY_HOME       0x84

#define VGA_MAX_LINES 25
#define VGA_MAX_COLS 80

#include <stdint.h>
#include <stdbool.h>
#include "string.h"



// VGA and keyboard state variables
extern volatile uint16_t* video_memory;
extern int vgaX;
extern int vgaY;

extern bool down_pressed;
extern bool up_pressed;
extern bool left_pressed;
extern bool right_pressed;
extern bool shift_pressed;
extern bool caps_lock_on;
extern bool ctrl_pressed;
extern bool alt_pressed;
extern bool keys[256];




void Start_Console();
String Console_Get_Command();


String get_string();
String get_string_after_index(int start);

#endif // CONSOLE_H
