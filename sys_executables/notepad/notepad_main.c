#include "../../src/headers/io.h"
#include "../../src/headers/console.h"
#include "../../src/headers/memory.h"

void app_main() {
    ClearScreen();

    #define INITIAL_STRING_SIZE 128
    #define MAX_STRING_LENGTH (VGA_MAX_LINES * VGA_MAX_COLS * 10)

    char* string = malloc(INITIAL_STRING_SIZE);
    if (!string) {
        printf("malloc failed\n");
        return;
    }

    string[0] = '\0';
    size_t string_size = INITIAL_STRING_SIZE;
    uint32_t cursor_index = 0;
    uint32_t input_start_line = 0;

    while (1) {
        unsigned char c = GetInputChar();

        // ESC to exit
        if (c == 27) break;

        // Backspace
        if (c == 0x08) {
            if (cursor_index > 0) {
                size_t len = strlen(string);
                for (size_t i = cursor_index - 1; i < len; ++i)
                    string[i] = string[i + 1];
                cursor_index--;
            }
        }
        // Printable characters
        else if (c >= 32 && c <= 126) {
            if (cursor_index + 1 >= string_size) {
                if (string_size * 2 > MAX_STRING_LENGTH) {
                    printf("max input reached\n");
                    continue;
                }
                size_t new_size = string_size * 2;
                char* new_string = malloc(new_size);
                if (!new_string) {
                    printf("realloc failed\n");
                    break;
                }
                strcpy(new_string, string);
                free(string);
                string = new_string;
                string_size = new_size;
            }

            size_t len = strlen(string);
            for (int i = len; i >= (int)cursor_index; i--)
                string[i + 1] = string[i];
            string[cursor_index] = c;
            cursor_index++;
        }
        // Arrow keys
        else if (c == KEY_LEFT && cursor_index > 0) {
            cursor_index--;
        } else if (c == KEY_RIGHT && cursor_index < strlen(string)) {
            cursor_index++;
        } else if (c == KEY_UP) {
            if (vgaY > input_start_line) {
                if (cursor_index >= VGA_MAX_COLS)
                    cursor_index -= VGA_MAX_COLS;
                else
                    cursor_index = 0;
            }
        } else if (c == KEY_DOWN) {
            cursor_index += VGA_MAX_COLS;
            if (cursor_index > strlen(string))
                cursor_index = strlen(string);
        }
        // Enter = newline
        else if (c == '\n') {
            size_t len = strlen(string);
            if (len + 1 < string_size) {
                for (int i = len; i >= (int)cursor_index; i--)
                    string[i + 1] = string[i];
                string[cursor_index] = '\n';
                cursor_index++;
            }
        }

        // === Print string with line wrapping ===
        int total_lines = 0;
        const char* scan = string;
        while (*scan) {
            if (*scan == '\n') total_lines++;
            scan++;
        }
        if (scan != string && *(scan - 1) != '\n') total_lines++;

        int start_line = total_lines > VGA_MAX_LINES ? total_lines - VGA_MAX_LINES : 0;
        int current_line = 0;
        const char* ptr = string;
        while (*ptr && current_line < VGA_MAX_LINES) {
            if (current_line >= start_line) {
                const char* line_start = ptr;
                const char* next_newline = strchr(ptr, '\n');
                int len = next_newline ? (next_newline - ptr) : strlen(ptr);

                char temp[128] = {0};
                strncpy(temp, line_start, len);
                printLine(temp, current_line - start_line);
            }
            const char* newline = strchr(ptr, '\n');
            if (!newline) break;
            ptr = newline + 1;
            current_line++;
        }

        // === Cursor repositioning ===
        int cursor_pos = cursor_index;
        vgaY = input_start_line + (cursor_pos / VGA_MAX_COLS);
        vgaX = cursor_pos % VGA_MAX_COLS;
    }

    free(string);
}
