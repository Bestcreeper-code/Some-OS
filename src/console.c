#include <stdint.h>
#include <stdbool.h>
#include "headers/console.h"
#include "headers/asm.h"
#include "headers/string.h"
#include "headers/ATA_IO.h"
#include "headers/time.h"
#include "headers/io.h"
#include "headers/memory.h"
#include "headers/multiboot_info.h"
#include "../FatFs/ff.h"
#include "headers/FileSystem.h"

#include "data/textconsts.h"

#define MAX_HISTORY 32

String command_History[MAX_HISTORY];
int command_History_count = 0;

extern volatile uint16_t* video_memory;
extern int vgaX, vgaY;

static char* currpath = 0;

// Keyboard state
extern bool shift_pressed, caps_lock_on, ctrl_pressed, alt_pressed;
#define VGA_MAX_LINES 25
#define VGA_MAX_COLS 80

String Console_Get_Command() {
    printf("%s>", currpath);
    char command_history_index = command_History_count;
    int start = strlen(currpath) + 1;  // prompt length + '>'
    int input_start_line = vgaY;

    String string = { .length = 0 };
    int cursor_index = 0;

    while (true) {
        unsigned char c = GetInputChar();

        int total_length = string.length + start;
        int lines_used = (total_length + VGA_MAX_COLS - 1) / VGA_MAX_COLS;

        // Prevent typing if screen filled except nav keys
        if (lines_used >= VGA_MAX_LINES &&
            c != '\n' && c != KEY_LEFT && c != KEY_RIGHT &&
            c != KEY_UP && c != KEY_DOWN) {
            continue;
        }

        // Scroll if at bottom line and new line or char input
        if ((vgaY == VGA_MAX_LINES - 1) &&
            (c == '\n' || (c != KEY_LEFT && c != KEY_RIGHT &&
             c != KEY_UP && c != KEY_DOWN))) {
            Scroll_Down();
            vgaY--;
            input_start_line--;
        }

        // Handle character input
        if (c == 0x08) { // Backspace
            if (cursor_index > 0) {
                // Remove char before cursor
                for (int i = cursor_index - 1; i < string.length - 1; i++) {
                    string.buffer[i] = string.buffer[i + 1];
                }
                string.length--;
                cursor_index--;
            }

        } else if (c >= 32 && c <= 126) { // Printable chars
            if (string.length < sizeof(string.buffer) - 1) {
                // Insert char at cursor position
                for (int i = string.length; i > cursor_index; i--) {
                    string.buffer[i] = string.buffer[i - 1];
                }
                string.buffer[cursor_index] = c;
                string.length++;
                cursor_index++;
            }

        } else if (c == KEY_LEFT) {
            if (cursor_index > 0) {
                cursor_index--;
            }

        } else if (c == KEY_RIGHT) {
            if (cursor_index < string.length) {
                cursor_index++;
            }

        } else if (c == KEY_UP || c == KEY_DOWN) {
            if (c == KEY_UP && command_history_index > 0) {
                command_history_index--;
            } else if (c == KEY_DOWN && command_history_index < 31) {
                command_history_index++;
            }

            if (command_history_index >= command_History_count) {
                string.length = 0;
                string.buffer[0] = '\0';
            } else {
                string = command_History[command_history_index];
            }
            cursor_index = string.length;

        } else if (c == '\n') {
            if (vgaY == VGA_MAX_LINES - 1) {
                Scroll_Down();
                if (input_start_line > 0) input_start_line--;
            } else {
                vgaY++;
            }
            move_cursor(0, vgaY);

            // Null terminate before return
            if (string.length < sizeof(string.buffer)) {
                string.buffer[string.length] = '\0';
            }
            return string;
        }

        // Clear lines used by input
        for (int i = 0; i <= lines_used && input_start_line + i < VGA_MAX_LINES; i++) {
            for (int j = 0; j < VGA_MAX_COLS; j++) {
                put_char(j, input_start_line + i, ' ', 0x0F);
            }
        }

        // Reprint prompt
        for (int i = 0; i < strlen(currpath); i++) {
            put_char(i, input_start_line, currpath[i], 0x0F);
        }
        put_char(strlen(currpath), input_start_line, '>', 0x0F);

        // Reprint input string with wrapping
        for (int i = 0; i < string.length; i++) {
            int pos = start + i;
            int line = input_start_line + (pos / VGA_MAX_COLS);
            int col = pos % VGA_MAX_COLS;
            if (line >= VGA_MAX_LINES) break;
            put_char(col, line, string.buffer[i], 0x0F);
        }

        // Move cursor to correct position
        int cursor_pos = start + cursor_index;
        vgaY = input_start_line + (cursor_pos / VGA_MAX_COLS);
        vgaX = cursor_pos % VGA_MAX_COLS;
        move_cursor(vgaX, vgaY);
    }
}








char* command_list[] = {
    "ping",
    "help",
    "cls",
    "dir",
    "echo",
    "read",
    "edit",
    "new",
    "rm",
    "time",
    NULL  // Null-terminated to mark the end
};


bool Console_Process_Command(String command) {
    int token_count = 0;
    bool result = true;

    char** tokens = Split(command.buffer, ' ', 2, &token_count);

    if (token_count <= 0) {
        result = false;
    } else if (!strcmp(tokens[0], "ping")) {
        printstr("pong\n");
    }
    else if (!strcmp(tokens[0], "help")) {
        printstr("Available commands:\n");
        for (int i = 0; command_list[i] != NULL; i++) {
            printstr(command_list[i]);
            printstr("\n");
        }
    }
    else if (!strcmp(tokens[0], "cls")) {
        ClearScreen();
    }
    else if (!strcmp(tokens[0], "dir")) {
        print_dir(currpath);
    }
    else if (!strcmp(tokens[0], "cd")) {
        if (token_count > 1) {
            if (change_Current_Dir(&currpath, tokens[1]) != FR_OK) {
                printf("Directory not found.\n");
            }
        } else {
            result = false;
        }
    }
    else if (!strcmp(tokens[0], "echo")) {
        if (token_count > 1) {
            printstr(tokens[1]);
            printstr("\n");
        } else {
            result = false;
        }
    }
    else if (!strcmp(tokens[0], "read")) {
        if (token_count > 1) {
            // Placeholder 
        } else {
            result = false;
        }
    }
    else if (!strcmp(tokens[0], "edit")) {
        if (token_count > 1) {
            // Placeholder 
            printstr("\n");
        } else {
            result = false;
        }
    }
    else if (!strcmp(tokens[0], "new")) {
        if (token_count < 2) {
            result = false;
        } else {
            
            printf("Sucessfully created %s\n", tokens[1]);
        }
    }
    else if (!strcmp(tokens[0], "mkdir")) {
        if (token_count < 2) {
            result = false;
        } else {
            char* list[2]={currpath,tokens[1]};
            if(f_mkdir(Concat(list,2,'/')) == FR_OK){
                printf("Sucessfully created directory %s\n", tokens[1]);
            } else {
                printf("Couldn't create directory %s\n", tokens[1]);
            }
        }
    }
    else if (!strcmp(tokens[0], "rm")) {
        if (token_count < 2) {
            result = false;
        } else {
            printf("Sucessfully removed %s\n", tokens[1]);
        }
    }
    else if (!strcmp(tokens[0], "time")) {
        rtc_time_t time;
        if (rtc_read_time(&time)) {
            printf("D/M/Y: %d/%d/%d\n", time.day, time.month, time.year);
            printf("%dH %dMin %dSec\n", time.hour, time.minute, time.second);
        }
    }
    else if (!strcmp(tokens[0], "run")) {
        FIL file;              // File object
        FRESULT res;           // Result code
        UINT bytesRead;        // Number of bytes read
        void* buffer;
        UINT fileSize;

        // Open the binary file for reading

        res = f_open(&file,"0:/gametest.bin", FA_READ);
        if (res == FR_OK) {
            fileSize = f_size(&file);  // Get file size

            // Allocate memory for file contents
            buffer = (void*)0x00200000; // Use a fixed address for simplicity
            if (buffer == NULL) {
                printf("Memory allocation failed.\n");
            } else {
                // Read the file contents into buffer
                res = f_read(&file, buffer, fileSize, &bytesRead);
                if (res == FR_OK && bytesRead == fileSize) {
                    // File successfully read into buffer
                    printf("File read successfully (%u bytes).\n", bytesRead);

                    // Define a function pointer to the entry point
                    void (*entry)(void) = (void (*)(void))buffer;

                    printf("Jumping to notepad.bin...\n");

                    // Call the loaded binary
                    entry();

                    // If it returns (unlikely), print something
                    printf("Returned from notepad.bin\n");
                } else {
                    printf("File read error: %d\n", res);
                    // free(buffer);
                }
            }

            f_close(&file);
        } else {
            printf("Failed to open file: %d\n", res);
        }
    }
    else {
        printstr("Unknown command. Type 'help' for a list of commands.\n");
        result = false;
    }



    EndSplit(tokens, token_count);
    return result;
}


void Start_Console() {
    currpath = malloc(4);
    if (!currpath) {
        printf("Path Broken");
    }
    strcpy(currpath,"0:/");
    ClearScreen();
    move_cursor(0, 0);
    printf(TitleAsciiString);
    

    
    while (true) {
        String command = Console_Get_Command();

        
        command_History[command_History_count] = command;
        command_History_count++;

        if (command_History_count > 31) {
            command_History_count = 31;
            
            for (char i = 1; i < 32; i++) {
                command_History[i - 1] = command_History[i];
            }
            command_History[31] = (String){0};
        }

        printstr("\n");
        Console_Process_Command(command);
    }

}

