#include "res.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/headers/console.h"
#include "../../src/headers/asm.h"
#include "../../src/headers/string.h"
#include "../../src/headers/ATA_IO.h"
#include "../../src/headers/time.h"
#include "../../src/headers/io.h"
#include "../../src/headers/memory.h"
#include "../../src/headers/multiboot_info.h"
#include "../../FatFs/ff.h"
#include "../../src/headers/FileSystem.h"

#include "../../src/data/textconsts.h"

#define MAX_HISTORY 32
#define MAX_COMMAND_LENGTH 256  // or adjust as needed

char* command_History[MAX_HISTORY];
int command_History_count = 0;

extern volatile uint16_t* video_memory;
extern short vgaX, vgaY;

static char* currpath = 0;

#define K_TERMINAL_WIDTH 25
#define K_TERMINAL_HEIGHT 80

// Allocate and return a malloc'd null-terminated string for the command
char* Console_Get_Command() {
    printf("%s>", currpath);
    int command_history_index = command_History_count;
    int start = strlen(currpath) + 1;  // prompt length + '>'
    int input_start_line = vgaY;

    char* buffer = malloc(MAX_COMMAND_LENGTH);
    if (!buffer) return NULL;
    int length = 0;
    int cursor_index = 0;

    while (true) {
        unsigned char c = getc();

        int total_length = length + start;
        int lines_used = (total_length + K_TERMINAL_HEIGHT - 1) / K_TERMINAL_HEIGHT;

        // Prevent typing if screen filled except nav keys
        if (lines_used >= K_TERMINAL_WIDTH &&
            c != '\n' && c != KEY_LEFT && c != KEY_RIGHT &&
            c != KEY_UP && c != KEY_DOWN) {
            continue;
        }

        // Scroll if at bottom line and new line or char input
        if ((vgaY == K_TERMINAL_WIDTH - 1) &&
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
            if (vgaY == K_TERMINAL_WIDTH - 1) {
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
        for (int i = 0; i <= lines_used && input_start_line + i < K_TERMINAL_WIDTH; i++) {
            for (int j = 0; j < K_TERMINAL_HEIGHT; j++) {
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
            int line = input_start_line + (pos / K_TERMINAL_HEIGHT);
            int col = pos % K_TERMINAL_HEIGHT;
            if (line >= K_TERMINAL_WIDTH) break;
            put_char(col, line, buffer[i], 0x0F);
        }

        // Move cursor to correct position
        int cursor_pos = start + cursor_index;
        vgaY = input_start_line + (cursor_pos / K_TERMINAL_HEIGHT);
        vgaX = cursor_pos % K_TERMINAL_HEIGHT;
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
            printf("Successfully created %s\n", tokens[1]);
        }
    }
    else if (!strcmp(tokens[0], "mkdir")) {
        if (token_count < 2) {
            result = false;
        } else {
            char* list[2] = {currpath, tokens[1]};
            if (f_mkdir(Concat(list, 2, '/')) == FR_OK) {
                printf("Successfully created directory %s\n", tokens[1]);
            } else {
                printf("Couldn't create directory %s\n", tokens[1]);
            }
        }
    }
    else if (!strcmp(tokens[0], "rm")) {
        if (token_count < 2) {
            result = false;
        } else {
            printf("Successfully removed %s\n", tokens[1]);
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
        if(token_count < 2) {
            result = false;
        } else {
            char* list[2] = {currpath, tokens[1]};
            // Load_bin_exe(Concat(list, 2, '\0'));
        }
    }
    else {
        printf("Unknown command %s. Type 'help' for a list of commands.\n", command);
        result = false;
    }

    EndSplit(tokens, token_count);
    return result;
}


void Start_Console() {
    currpath = malloc(4);
    if (!currpath) {
        printf("Path Broken");
        return;
    }
    strcpy(currpath, "0:/");
    ClearScreen();
    move_cursor(0, 0);

    set_print_color(0x8);
    printf(TitleAsciiString);
    printf("\n");
    printf("%d", Get_multiboot_info()->framebuffer_pitch);
    set_print_color(0x0F);

    while (true) {
        char* command = Console_Get_Command();
        if (!command) continue;

        // Add command to history
        if (command_History_count == MAX_HISTORY) {
            // Free oldest
            free(command_History[0]);
            // Shift all left
            for (int i = 1; i < MAX_HISTORY; i++) {
                command_History[i - 1] = command_History[i];
            }
            command_History_count--;
        }

        command_History[command_History_count++] = command;

        printstr("\n");
        Console_Process_Command(command);
    }
}
