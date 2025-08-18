#include "res.h"
#include "../../src/headers/string.h"
#include "../../src/headers/io.h"
#include "../../src/headers/memory.h"
#include "../../src/headers/addresses.h"
#include "../../src/headers/FileSystem.h"
#include "../../src/data/textconsts.h"
#include <stddef.h>

// Include your kernel headers here, e.g.,
// #include "headers/console.h"
// externs for video memory, VGA cursor position, etc.

// Your Console_Get_Command() and related input handling here,
// plus command history, currpath, etc.

#define MAX_HISTORY 32
#define MAX_COMMAND_LENGTH 256



// Example Console_Get_Command() prototype here, implement your input reading loop as before


void app_main() {
    fs_set((FATFS*)FATFS_SYS_ADDR,0);//needed otherwise no fatfs func works
    Start_Console();
}
