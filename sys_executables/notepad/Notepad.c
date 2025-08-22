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

#define CHARS_PER_LINE 15
#define DISPLAYED_LINES 15

#define TEXT_AREA_X 6
#define TEXT_AREA_Y 32
#define TEXT_AREA_W 306
#define TEXT_AREA_H 160

#define DEFAULT_FILE_STRING "EMPTY FILE" 

char font_w = 4;
char font_h = 6;


void app_main(int argc, char** argv) {
    fs_set((FATFS*)FATFS_SYS_ADDR,0);//NEEDED FOR FILE SYS ACCESS
    printf("%s",argv[1]);
    sleep(1000);
    vga_set_mode(0X13);
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

    size = strlen(DEFAULT_FILE_STRING);
    data = malloc(size + 1);
    if (data != NULL) {
        memcpy(data, DEFAULT_FILE_STRING, size + 1);
    }

done:
    int scroll_index = 0;

    int cursor_index = 10;

    clear_13h_screen(0);//black outline
    Draw_Rect((Vector2){1,1}, 318, 198, 0x09);//blue outline/tool bar
    Draw_Rect((Vector2){TEXT_AREA_X-1,TEXT_AREA_Y-1}, TEXT_AREA_W+2, TEXT_AREA_H+2, 0);//2nd black outline
    unsigned char input = 0;
    int line_count;
    char** lines = split_text_lines(data, CHARS_PER_LINE, &line_count);
    bool data_changed = false;
    
    while (1){
        input = getc();

        switch (input){
            case KEY_ENTER:
                size++;
                char* tmp = realloc(data, size+1);
                if (tmp != NULL) data = tmp;
                InsertChar(data ,cursor_index,'\n');
                cursor_index++;
                data_changed = true;
                break;

            default:
                if(input <= 127 && input >= 32){
                    size++;
                    char* tmp = realloc(data, size+1);
                    if (tmp != NULL) data = tmp;
                    InsertChar(data ,cursor_index,input);
                    cursor_index++;
                    data_changed = true;
                }
                break;
                
        }
            
        if (data_changed)lines = split_text_lines(data, CHARS_PER_LINE, &line_count);

        Draw_Rect((Vector2){TEXT_AREA_X, TEXT_AREA_Y}, TEXT_AREA_W, TEXT_AREA_H, 0x37); // clear text area

        int char_counter = 0; 

        for (int i = 0; i < DISPLAYED_LINES; i++) {
            int line_index = scroll_index + i;
            if (line_index >= line_count) break;

            char* line = lines[line_index];

            for (int j = 0; j < strlen(line); j++) {
                unsigned char charact = line[j];
                uint16_t charx = TEXT_AREA_X + 1 + ((font_w + 1) * j);
                uint8_t chary = TEXT_AREA_Y + 1 + (font_h * i);

                // Draw cursor
                
                draw_bitmap_char(charact, charx, chary, font_w, font_h, 0, NULL, true);

                if (cursor_index == char_counter) {
                    draw_bitmap_char('|', charx - 2, chary, font_w, font_h, 0x9, NULL, true);
                }

                char_counter++;
            }

            // Handle newline as a character (even if we don't draw it)
            char_counter++;
        }

        // Draw cursor at end of text if needed
        if (cursor_index == char_counter) {
            int cursor_line = char_counter / CHARS_PER_LINE;
            int cursor_col = char_counter % CHARS_PER_LINE;

            if (cursor_line >= scroll_index && cursor_line < scroll_index + DISPLAYED_LINES) {
                int i = cursor_line - scroll_index;
                uint16_t charx = TEXT_AREA_X + 1 + ((font_w + 1) * cursor_col);
                uint8_t chary = TEXT_AREA_Y + 1 + (font_h * i);
                draw_bitmap_char('|', charx - 1, chary, font_w, font_h, 0x2, NULL, true);
            }
        }

    }

    free(data);
    sleep(10000);
}
