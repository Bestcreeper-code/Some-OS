#include "../../src/headers/io.h"
#include "../../src/headers/video.h"
#include "../../src/headers/memory.h"
#include "../../src/headers/FileSystem.h"
#include "../../src/headers/time.h"
#include "../../src/headers/random.h"
#include "../../src/headers/vga_modes.h"
#include "Graphics/graphics.h"
#include "res.h"
#include "../../src/data/globals.h"
#include "../../src/data/textconsts.h"

#define CHARS_PER_LINE 60
#define DISPLAYED_LINES 15

#define TITLE_LINE_Y 0
#define OPTIONS_LINE_Y 1
#define TEXT_START_LINE_Y 2 // Content starts at line 2 (3rd row)
#define PADDING_LEFT 5

#define DEFAULT_FILE_STRING "EMPTY FILE"

char font_w = 4;
char font_h = 6;

void app_main(int argc, char** argv) {
    fs_set((FATFS*)FATFS_SYS_ADDR, 0); // Initialize filesystem
    printf("%s", argv[1]);
    sleep(1000);

    vga_set_mode(0x03); // Set VGA text mode

    char* data = NULL;
    int size = 0;
    FIL file;
    UINT br;

    if (argc > 1 && check_path_exists(argv[1], FT_FILE) == FR_OK
        && f_open(&file, argv[1], FA_READ) == FR_OK) {
        
        size = f_size(&file);
        data = malloc(size + 1);
        if (data != NULL) {
            if (f_read(&file, data, size, &br) == FR_OK && br == size) {
                data[size] = '\0';
                f_close(&file);
                goto done;
            }
            free(data);
            data = NULL;
        }
        f_close(&file);
    }

    // If file failed to load
    size = strlen(DEFAULT_FILE_STRING);
    data = malloc(size + 1);
    if (data != NULL) {
        memcpy(data, DEFAULT_FILE_STRING, size + 1);
    }

done:
    int scroll_index = 0;
    int cursor_index = 0;

    ClearScreen();

    // Draw static UI elements (title + options)
    const char* title = " Simple Text Editor ";
    for (int i = 0; i < strlen(title); i++) {
        put_char(i + 1, TITLE_LINE_Y + 1, title[i], 0x1F); // white on blue
    }

    const char* options = "[F1] Save   [ESC] Exit";
    for (int i = 0; i < strlen(options); i++) {
        put_char(i + 1, OPTIONS_LINE_Y + 1, options[i], 0x2E); // light green on black
    }

    unsigned char input = 0;
    int line_count;
    char** lines = split_text_lines(data, CHARS_PER_LINE, &line_count);
    bool data_changed = false;

    while (1) {
        input = getc();

        switch (input) {
            case KEY_ENTER:
                size++;
                {
                    char* tmp = realloc(data, size + 1);
                    if (tmp != NULL) data = tmp;
                    InsertChar(data, cursor_index, '\n');
                    cursor_index++;
                    data_changed = true;
                }
                break;

            case KEY_BACKSPACE:
                if (cursor_index > 0) {
                    size--;
                    memmove(&data[cursor_index - 1], &data[cursor_index], size - cursor_index + 1);
                    cursor_index--;
                    size = size < 0 ? 0 : size;
                    data_changed = true;
                }
                break;
            case KEY_LEFT:
                if (cursor_index > 0) cursor_index--;
                break;
            case KEY_RIGHT:
                if (cursor_index < size) cursor_index++;
                break;
                
                

            default:
                if (input <= 127 && input >= 32) {
                    size++;
                    char* tmp = realloc(data, size + 1);
                    if (tmp != NULL) data = tmp;
                    InsertChar(data, cursor_index, input);
                    cursor_index++;
                    data_changed = true;
                }
                break;
        }

        if (data_changed) {
            free(lines);
            lines = split_text_lines(data, CHARS_PER_LINE, &line_count);
            data_changed = false;
        }

        // Clamp scroll_index
        if (scroll_index > line_count - DISPLAYED_LINES) {
            scroll_index = line_count - DISPLAYED_LINES;
            if (scroll_index < 0) scroll_index = 0;
        }
        if (scroll_index < 0) scroll_index = 0;

        // Compute cursor's line and column in the split lines
        int running_index = 0;
        int cursor_line = 0;
        int cursor_col = 0;
        for (int i = 0; i < line_count; i++) {
            int line_len = strlen(lines[i]);
            if (cursor_index <= running_index + line_len) {
                cursor_line = i;
                cursor_col = cursor_index - running_index;
                break;
            }
            running_index += line_len + 1; // +1 for newline char
        }

        // Adjust scroll to keep cursor visible on screen
        if (cursor_line < scroll_index) {
            scroll_index = cursor_line;
        } else if (cursor_line >= scroll_index + DISPLAYED_LINES) {
            scroll_index = cursor_line - DISPLAYED_LINES + 1;
        }

        // Redraw visible lines only
        for (int i = 0; i < DISPLAYED_LINES; i++) {
            int line_index = scroll_index + i;
            if (line_index >= line_count) {
                // Clear leftover lines
                for (int col = 0; col < CHARS_PER_LINE + PADDING_LEFT + 5; col++) {
                    put_char(col + 1, TEXT_START_LINE_Y + i + 1, ' ', 0x07);
                }
                continue;
            }

            char* line = lines[line_index];

            // Draw left margin: line number + '>'
            char tempstr[6];
            sprintf(tempstr, "%04d>", line_index + 1);
            for (int j = 0; j < 5; j++) {
                put_char(j + 1, TEXT_START_LINE_Y + i + 1, tempstr[j], 0x0F);
            }

            // Draw line content
            int line_len = strlen(line);
            for (int j = 0; j < line_len; j++) {
                put_char(j + 1 + PADDING_LEFT, TEXT_START_LINE_Y + i + 1, line[j], 0x3F);
            }
            // Clear rest of line if shorter than CHARS_PER_LINE
            for (int j = line_len; j < CHARS_PER_LINE; j++) {
                put_char(j + 1 + PADDING_LEFT, TEXT_START_LINE_Y + i + 1, ' ', 0x07);
            }
        }

        // Calculate visible cursor position (relative to scroll)
        int visible_line = cursor_line+1 - scroll_index;
        if (visible_line < 0) visible_line = 0;
        if (visible_line >= DISPLAYED_LINES) visible_line = DISPLAYED_LINES - 1;

        move_cursor(cursor_col + 1 + PADDING_LEFT, visible_line + 1 + TEXT_START_LINE_Y);
    }

    free(data);
    free(lines);
    sleep(10000);
}
