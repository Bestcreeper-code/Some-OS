#include "console.h"


#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "asm.h"
#include "cpu/cpu.h"
#include "drivers.h"
#include "err_codes.h"
#include "string.h"
#include "ATA_IO.h"
#include "time.h"
#include "io.h"
#include "memory.h"

#include "../FatFs/ff.h"
#include "FileSystem.h"

#include "data/textconsts.h"
#include "paging.h"

#define MAX_HISTORY 32
#define MAX_COMMAND_LENGTH 256  

char* command_History[MAX_HISTORY];
int command_History_count = 0;

extern volatile uint16_t* video_memory;
extern int vgaX, vgaY;

static char* currpath = 0;

// #define K_TERMINAL_WIDTH K_TERMINAL_WIDTH
// #define K_TERMINAL_HEIGHT K_TERMINAL_HEIGHT

// Allocate and return a kmalloc'd null-terminated string for the command
char* Console_Get_Command() {
    
    printf("%s> ", currpath);
    int command_history_index = command_History_count;
    int start = strlen(currpath) + 2;  // path length + ""> ""
    int input_start_line = vgaY;

    char* buffer = kmalloc(MAX_COMMAND_LENGTH);
    
    if (!buffer) return NULL;
    
    int length = 0;
    int cursor_index = 0;

    while (true) {
        unsigned char c = getc();

        int total_length = length + start;
        int lines_used = (total_length + K_TERMINAL_WIDTH - 1) / K_TERMINAL_WIDTH;

        // Prevent typing if screen filled except nav keys
        if (lines_used >= K_TERMINAL_HEIGHT &&
            c != '\n' && c != KEY_LEFT && c != KEY_RIGHT &&
            c != KEY_UP && c != KEY_DOWN) {
            continue;
        }

        // Scroll if at bottom line and new line or char input
        if ((vgaY == K_TERMINAL_HEIGHT - 1) &&
            (c == '\n' || (c != KEY_LEFT && c != KEY_RIGHT &&
             c != KEY_UP && c != KEY_DOWN))) {
            Scroll_Down();
            vgaY--;
            input_start_line--;
        }

        // Handle character input
        if (c == 0x08) { // Backspace
            if (cursor_index > 0) {
                for (int i = cursor_index - 1; i < length - 1; i++) {
                    buffer[i] = buffer[i + 1];
                }
                length--;
                cursor_index--;
            }

        } else if (c >= 32 && c <= 126) { // Printable chars
            if (length < MAX_COMMAND_LENGTH - 1) {
                for (int i = length; i > cursor_index; i--) {
                    buffer[i] = buffer[i - 1];
                }
                buffer[cursor_index] = c;
                length++;
                cursor_index++;
            }

        } else if (c == KEY_LEFT) {
            if (cursor_index > 0) {
                cursor_index--;
            }

        } else if (c == KEY_RIGHT) {
            if (cursor_index < length) {
                cursor_index++;
            }

        } else if (c == KEY_UP || c == KEY_DOWN) {
            if (c == KEY_UP && command_history_index > 0) {
                command_history_index--;
            } else if (c == KEY_DOWN && command_history_index < MAX_HISTORY - 1) {
                command_history_index++;
            }

            if (command_history_index >= command_History_count) {
                length = 0;
                buffer[0] = '\0';
            } else {
                const char* hist = command_History[command_history_index];
                length = strlen(hist);
                if (length >= MAX_COMMAND_LENGTH) length = MAX_COMMAND_LENGTH - 1;
                memcpy(buffer, hist, length);
                buffer[length] = '\0';
            }
            cursor_index = length;

        } else if (c == '\n') {
            if (vgaY == K_TERMINAL_HEIGHT - 1) {
                Scroll_Down();
                if (input_start_line > 0) input_start_line--;
            } else {
                vgaY++;
            }
            move_cursor(0, vgaY);

            buffer[length] = '\0';
            return buffer;
        }

        // Clear lines used by input
        for (int i = 0; i < lines_used && input_start_line + i < K_TERMINAL_HEIGHT; i++) {
            for (int j = 0; j < K_TERMINAL_WIDTH; j++) {
                put_char(j, input_start_line + i, ' ', 0x0F);
            }
        }

        // Reprint prompt
        for (int i = 0; i < (int)strlen(currpath); i++) {
            put_char(i, input_start_line, currpath[i], 0x0F);
        }
        put_char(strlen(currpath), input_start_line, '>', 0x0F);

        // Reprint input string with wrapping
        for (int i = 0; i < length; i++) {
            int pos = start + i;
            int line = input_start_line + (pos / K_TERMINAL_WIDTH);
            int col  = pos % K_TERMINAL_WIDTH;
            if (line >= K_TERMINAL_HEIGHT) break;
            put_char(col, line, buffer[i], 0x0F);
        }

        // Move cursor to correct position
        int cursor_pos = start + cursor_index;
        vgaY = input_start_line + (cursor_pos / K_TERMINAL_WIDTH);
        if (vgaY >= K_TERMINAL_HEIGHT) vgaY = K_TERMINAL_HEIGHT - 1;
        vgaX = cursor_pos % K_TERMINAL_WIDTH;
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
    NULL
};

bool Console_Process_Command(char* command) {
    int token_count = 0;
    bool result = true;
    if (command == NULL || strlen(command) == 0) {
        return false;
    }
    bool echoing = true;
    if(command[0] == '@'){
        echoing = false;
        command++; 
    }

    char** tokens = Split(command, ' ', 2, &token_count);

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
    // else if (!strcmp(tokens[0], "dir")) {
    //     print_dir(currpath);
    // }
    // else if (!strcmp(tokens[0], "cd")) {
    //     if (token_count > 1) {
    //         if (change_Current_Dir(&currpath, tokens[1]) != FR_OK) {
    //             printf("Directory not found.\n");
    //         }
    //     } else {
    //         result = false;
    //     }
    // }
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
    else if (!strcmp(tokens[0], "cpu")) {
        cpu_log_specs();
    }
    else if (!strcmp(tokens[0], "zerodiv")) {
        int e = 8839/0;
    }
    else if (!strcmp(tokens[0], "nullptr")) {
        char* wtf= 0;
        token_count = *wtf++;
    }
    else if (!strcmp(tokens[0], "rm")) {
        if (token_count < 2) {
            result = false;
        } else {
            if (echoing) printf("Successfully removed %s\n", tokens[1]);
        }
    }
    else if (!strcmp(tokens[0], "time")) {
        rtc_time_t time;
        if (rtc_read_time(&time)) {
            if (echoing) {
                printf("D/M/Y: %d/%d/%d\n", time.day, time.month, time.year);
                printf("%dH %dMin %dSec\n", time.hour, time.minute, time.second);
            }
        }
    }
    
    else if (!strcmp(tokens[0], "mem")) {
        char buffer[16];
        byte_nb_simplify(get_used_ram(),buffer);
        printf("mem=  %s",buffer);
        
        byte_nb_simplify(ram_amount,buffer);
        printf(" / %s\n",buffer);

    }
    else {
        if (echoing) printf("Unknown command %s. Type 'help' for a list of commands.\n", command);
        result = false;
    }

    EndSplit(tokens, token_count);
    return result;
}


char* console_requests[32];

REGISTER_DRIVER_LATE(kconsole, Start_Console);
int Start_Console() {
    Sys_log("Kernel Console started\n");
    memset(CONSOLE_REQUEST_QUEUE, 0, sizeof(char*) * 16);
    // Add_Console_Request("@help");
    // int i;
    // i/=0;
    currpath = kmalloc(4);
    if (!currpath) {
        printf("Path Broken");
        RET_ERR(E_NOMEM);
    }
    strcpy(currpath, "0:/");
    ClearScreen();
    move_cursor(0, 0);

    set_print_color(0x8);
    printf(TitleAsciiString);
    printf("\n");
    set_print_color(0xF);

    while (true) {
        char* command;
        bool usr_input = true;
        if(console_requests[0] == 0)command = Console_Get_Command();
        else{
            command = console_requests[0];
            memmove(&console_requests[0], &console_requests[1], sizeof(char*) * 15);
            console_requests[15] = NULL;
            usr_input = false;
        }
        if (!command) continue;

        // Add command to history
        if (command_History_count == MAX_HISTORY && usr_input) {
            // Free oldest
            kfree(command_History[0]);
            // Shift all left
            for (int i = 1; i < MAX_HISTORY; i++) {
                command_History[i - 1] = command_History[i];
            }
            command_History_count--;
        }

        if(usr_input) command_History[command_History_count++] = command;

        printstr("\n");
        Console_Process_Command(command);
    }
}

char Add_Console_Request(char* command){
    for(int i = 0; i < CONSOLE_REQUEST_QUEUE_SIZE; i++){
        if(console_requests[i] == NULL){
            console_requests[i] = command;
            return 1;
        }
    }
    return 0;
}