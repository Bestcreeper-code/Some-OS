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
#define DISPLAYED_LINES 25

#define TITLE_LINE_Y 0
#define OPTIONS_LINE_Y 1
#define TEXT_START_LINE_Y 4 // Content starts at line 2 (3rd row)
#define PADDING_LEFT 5

#define DEFAULT_FILE_STRING "EMPTY FILE"

char font_w = 4;
char font_h = 6;

void app_main(int argc, char** argv) {
    fs_set((FATFS*)FATFS_SYS_ADDR, 0); // Initialize filesystem
    vga_set_mode(0x13); // 320x200x256 graphics mode
    clear_13h_screen(0x00); // Clear screen (assuming you have this)

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

    // Fallback text if file fails
    size = strlen(DEFAULT_FILE_STRING);
    data = malloc(size + 1);
    if (data != NULL) {
        memcpy(data, DEFAULT_FILE_STRING, size + 1);
    }

done:
    if (!data) return;

    int total_lines = 0;
    char** lines = split_text_lines(data, CHARS_PER_LINE, &total_lines);
    int scroll_offset = 0;

    while (1) {
        // Render
        clear_13h_screen(0x00); // Clear screen each frame

        // Title
        draw_bitmap_string( "Text Viewer",PADDING_LEFT, TITLE_LINE_Y * font_h, font_w, font_h, 0x3F,NULL,true,false,0);

        // Filename (optional)
        if (argc > 1) {
            draw_bitmap_string( argv[1],PADDING_LEFT, OPTIONS_LINE_Y * font_h, font_w, font_h, 0x3F,NULL,true,false,0);
        }

        // Draw visible lines
        for (int i = 0; i < DISPLAYED_LINES; i++) {
            int line_index = scroll_offset + i;
            if (line_index < total_lines) {
                draw_bitmap_string( lines[line_index], PADDING_LEFT, (TEXT_START_LINE_Y + i) * font_h, font_w, font_h, 0x3F,NULL,true,false,0);
            }
        }

        // Handle input
        unsigned char key = getc(); // or non-blocking with delay
        if (key == KEY_UP && scroll_offset > 0) {
            scroll_offset--;
        } else if (key == KEY_DOWN && scroll_offset + DISPLAYED_LINES < total_lines) {
            scroll_offset++;
        } else if (key == KEY_ESCAPE) {
            break; // exit viewer
        }
    }

    // Cleanup
    for (int i = 0; i < total_lines; i++) {
        free(lines[i]);
    }
    free(lines);
    free(data);
}
