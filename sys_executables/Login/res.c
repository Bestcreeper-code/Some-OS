#include "Graphics/graphics.h"
#include "../../src/headers/video.h"
#include <stddef.h>
#include "../../src/headers/io.h"
#include "../../src/headers/memory.h"
#include "../../src/headers/FileSystem.h"
#include "../../FatFs/ff.h"

char* intToStr(int num) {
    // Handle zero as special case
    if (num == 0) {
        char* zeroStr = malloc(2);
        if (!zeroStr) return NULL;
        zeroStr[0] = '0';
        zeroStr[1] = '\0';
        return zeroStr;
    }

    int n = num;
    int length = 0;

    // Count digits
    while (n > 0) {
        n /= 10;
        length++;
    }

    // Allocate string (+1 for null terminator)
    char* str = malloc(length + 1);
    if (!str) return NULL;

    str[length] = '\0';  // Null-terminate
    int i = length - 1;

    // Fill string from the end
    while (num > 0) {
        str[i--] = (num % 10) + '0';
        num /= 10;
    }

    return str;
}



#define MAX_STRING_INPUT_LEN 256


char* String_Input_Popup(int x, int y,int width, bool hidden) {
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
        if(!hidden) draw_bitmap_string(visible_str, x, y, font_w, font_h, 0X3F, NULL, true, space);
        else {
            int hid_len = strlen(visible_str);
            char* hidden_str = malloc(hid_len + 1);
            if (!hidden_str) {
                free(buffer);
                return NULL;
            }
            for (int i = 0; i < hid_len; i++) {
                hidden_str[i] = '*';
            }
            hidden_str[hid_len] = '\0';
            draw_bitmap_string(hidden_str, x, y, font_w, font_h, 0X3F, NULL, true, space);
            free(hidden_str);
        }

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

#define F_EDIT_POPUP_POS_X            80
#define F_EDIT_POPUP_POS_Y            30

#define F_EDIT_POPUP_M_DISPL_ENTRIES  3

#define F_EDIT_POPUP_COMMANDS_AMOUNT  3
const char* edit_popup_content[F_EDIT_POPUP_COMMANDS_AMOUNT] = {
    "RENAME",
    "MOVE",
    "DELETE"
};

uint8_t Process_File_Edit(char* path,char action){
    switch (action)
    {
    case 0: //RENAME
    {
        char* input = String_Input_Popup(125, 94, 70, false);
        if(input == NULL)break;

        char* new_filename = Get_Filename(input);
        char* dir = Get_Dir(path);

        char* prts[2] = {dir, new_filename };

        char* newpath = Concat(prts,2,'/');

        f_rename(path, newpath);

        free(input);
        free(new_filename);
        free(dir);
        free(newpath);

        break;
    }
    case 1: //MOVE
    {
        char* input = String_Input_Popup(125, 94, 70, false);
        if(input == NULL)break;

        char* filename = Get_Filename(path);
        char* curr_dir = Get_Dir(path);
        

        if(change_Current_Dir(&curr_dir, input) == FR_OK){

            char* prts[2] = { curr_dir, filename };

            char* newpath = Concat(prts,2,'/');
            f_rename(path, newpath);
            free(newpath);
        }

        free(input);
        free(filename);
        free(curr_dir);

        break;
    }
    case 2: //DELETE
    {
        return f_unlink(path);
    }
    default:
        break;
    }
    
}


uint8_t Open_File_Edit_Popup(char* file) {
    uint8_t cursor_index = 0;
    uint8_t scroll_index = 0;

    while (true) {

        Draw_Rect((Vector2){F_EDIT_POPUP_POS_X-2, F_EDIT_POPUP_POS_Y-2}, 164 + 5, 64 + 5, 0x0);
        Draw_Rect((Vector2){F_EDIT_POPUP_POS_X, F_EDIT_POPUP_POS_Y}, 160, 60, 0x26);


        for (uint8_t i = 0; i < F_EDIT_POPUP_M_DISPL_ENTRIES; i++) {
            uint8_t item_index = scroll_index + i;
            if (item_index >= F_EDIT_POPUP_COMMANDS_AMOUNT) break;


            uint8_t color = (i == cursor_index) ? 0x3F : 0x1F;

            draw_bitmap_string(
                edit_popup_content[item_index],
                F_EDIT_POPUP_POS_X + 4,
                F_EDIT_POPUP_POS_Y + 6 + (i * 10),
                4, 6,
                color,
                NULL, true, 0
            );
        }


        unsigned char input = getc();

        switch (input) {
            case KEY_UP:
                if (cursor_index > 0) {
                    cursor_index--;
                } else if (scroll_index > 0) {
                    scroll_index--;
                }
                break;

            case KEY_DOWN:
                if ((cursor_index + scroll_index + 1) < F_EDIT_POPUP_COMMANDS_AMOUNT) {
                    if (cursor_index < F_EDIT_POPUP_M_DISPL_ENTRIES - 1) {
                        cursor_index++;
                    } else {
                        scroll_index++;
                    }
                }
                break;

            case KEY_ENTER:
                return Process_File_Edit(file,scroll_index + cursor_index);                

            case KEY_ESCAPE:
                return 0; 

            default:
                break;
        }
    }
}

char* xor_crypt(const char* value, int value_size, const char* key, int key_size) {
    char* output = malloc(value_size);
    if (!output) return NULL;

    for (int i = 0; i < value_size; i++) {
        output[i] = value[i] ^ key[i % key_size];
    }
    return output;
}

    
