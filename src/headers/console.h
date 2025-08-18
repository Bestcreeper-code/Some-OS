#ifndef CONSOLE_H
#define CONSOLE_H




#define VGA_MAX_LINES 25
#define VGA_MAX_COLS 80
#define MAX_HISTORY 32
#define MAX_COMMAND_LENGTH 256

#include <stdint.h>
#include <stdbool.h>
#include "string.h"



// VGA and keyboard state variables
extern volatile uint16_t* video_memory;
extern int vgaX;
extern int vgaY;





void Start_Console();
char* Console_Get_Command();




#endif // CONSOLE_H
