#include "Graphics/graphics.h"
#include "../../src/headers/video.h"
#include <stddef.h>
#include "../../src/headers/io.h"
#include "../../FatFs/ff.h"
void intToStr(int num, char* str) {
    int i = 0;

    // Handle zero explicitly
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    // Extract digits in reverse order
    while (num > 0) {
        int digit = num % 10;
        str[i++] = digit + '0'; // Convert digit to char
        num /= 10;
    }
    str[i] = '\0';

    // Reverse the string because digits are in reverse
    for (int j = 0; j < i / 2; j++) {
        char tmp = str[j];
        str[j] = str[i - 1 - j];
        str[i - 1 - j] = tmp;
    }
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
        /* placeholder */
        break;
    case 1: //MOVE
        /* placeholder */
        break;
    case 2: //DELETE
        f_unlink(path);
        break;
    
    default:
        break;
    }
}

uint8_t Open_File_Edit_Popup(char* file) {
    uint8_t cursor_index = 0;
    uint8_t scroll_index = 0;

    while (true) {

        Draw_Rect((Vector2){F_EDIT_POPUP_POS_X-2, F_EDIT_POPUP_POS_Y-2}, 164, 64, 0x0);
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

