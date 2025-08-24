#include "Graphics/graphics.h"
#include "../../src/headers/video.h"
#include <stddef.h>
#include "../../src/headers/io.h"
#include "../../src/headers/memory.h"
#include "../../src/headers/FileSystem.h"
#include "../../FatFs/ff.h"



#define MAX_STRING_INPUT_LEN 256


char* String_Input_Popup(int x, int y,int width) {
    char color = 0x9;
    int font_w = 4, font_h = 6, space = 1;
    
    int height = font_h+2;

    int max_rendered_chars = width / (font_w + space);

    char* buffer = (char*)malloc(MAX_STRING_INPUT_LEN);
    if (!buffer) return NULL;

    int len = 0;
    buffer[0] = '\0';

    while (1) {
        Draw_Rect((Vector2){x - 2, y - 2}, width + 9, height + 9, 0); // black shadow 
        Draw_Rect((Vector2){x, y}, width, height, color); // input area

        char* visible_str = buffer;
        if (len > max_rendered_chars) {
            visible_str = buffer + (len - max_rendered_chars);
        }

        // String inside the box
        draw_bitmap_string(visible_str, x, y, font_w, font_h, 0X3F, NULL, true, space);

        // Get input
        char ch = getc();

        switch (ch) {
            case KEY_ENTER:
                return buffer;

            case KEY_BACKSPACE:
                if (len > 0) {
                    len--;
                    buffer[len] = '\0';
                }
                break;
            case KEY_ESCAPE:
                return NULL;


            default:
                if (ch >= 32 && ch <= 126 && len < MAX_STRING_INPUT_LEN - 1) {
                    buffer[len++] = ch;
                    buffer[len] = '\0';
                }
                break;
        }
    }

    return buffer;
}

char** split_text_lines(const char* text,int char_per_line, int* out_line_count) {
    int capacity = 16;
    int count = 0;
    char** lines = malloc(sizeof(char*) * capacity);

    int text_len = strlen(text);
    int i = 0;

    while (i < text_len) {
        char buffer[char_per_line + 1];
        int buf_index = 0;

        while (buf_index < char_per_line && i < text_len) {
            if (text[i] == '\n') {
                i++; // skip newline
                break; // end line early
            }
            buffer[buf_index++] = text[i++];
        }

        buffer[buf_index] = '\0';

        if (count >= capacity) {
            capacity *= 2;
            lines = realloc(lines, sizeof(char*) * capacity);
        }

        lines[count++] = strdup(buffer);
    }

    *out_line_count = count;
    return lines;
}

