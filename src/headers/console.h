#ifndef CONSOLE_H
#define CONSOLE_H




// #define K_TERMINAL_WIDTH 25
// #define K_TERMINAL_HEIGHT 80
#define MAX_HISTORY 32
#define MAX_COMMAND_LENGTH 256

#include <stdint.h>
#include <stdbool.h>
#include "string.h"
#include "io.h"



// VGA and keyboard state variables
extern volatile uint16_t* video_memory;
extern int vgaX;
extern int vgaY;





void Start_Console();
char* Console_Get_Command();

char Add_Console_Request(char* command);


#endif // CONSOLE_H
