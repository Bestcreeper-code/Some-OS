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
//y capped to 125 smh
#define MAX_DISPLAYED_ENTRIES 15


void app_main(int argc, char** argv) {
    char* base_path = "0:/";
    if(argc > 1)base_path = argv[1];
    fs_set((FATFS*)FATFS_SYS_ADDR,0);
    char* current_dir = strdup(base_path);

    char** dir_content = NULL;
    int dir_content_size = 0;

    int cursor_index;
    int scroll_index;

    
    
    vga_set_mode(0x13);

change_dir:
    cursor_index = 0;
    scroll_index = 0;

    if(dir_content) {
        for(int i = 0; i < dir_content_size; i++) {
            free(dir_content[i]);
        }
        free(dir_content);
    }


    dir_content = read_dir(current_dir,&dir_content_size);
    clear_13h_screen(0x9);
    Draw_Rect((Vector2){8, 8}, 303, 108,0x3);//white outline
    Draw_Rect((Vector2){12, 12}, 295, 100,0x9);// actual dir display area 

    draw_bitmap_string(current_dir,0,0,4,6,0x12,NULL,true,1);

    uint8_t input = 0;

    while (true)
    {

        switch (input)
        {
        case KEY_UP:
            if(cursor_index <= 0){
                if(scroll_index > 0){
                    scroll_index--;
                }
                cursor_index=0;
            } else cursor_index--;
            break;
        case KEY_DOWN:
            if(cursor_index >= MAX_DISPLAYED_ENTRIES-1 && cursor_index + scroll_index < dir_content_size){
                scroll_index++;
                if(scroll_index + MAX_DISPLAYED_ENTRIES > dir_content_size)
                    scroll_index = dir_content_size - MAX_DISPLAYED_ENTRIES;
                if(scroll_index < 0)
                    scroll_index = 0;
                cursor_index = MAX_DISPLAYED_ENTRIES-1;
            } else {
                cursor_index++;
                if(cursor_index >= dir_content_size)
                    cursor_index = dir_content_size - 1;
            }
            break;
        case KEY_BACKSPACE:
            if(change_Current_Dir(&current_dir,"../") == FR_OK) goto change_dir;
            break;
        case KEY_ENTER:
            if(change_Current_Dir(&current_dir,dir_content[scroll_index + cursor_index]) == FR_OK) goto change_dir;
            break;
        // case KEY_HOME:
        //     if(strcpy(current_dir,"0:/") == FR_OK) goto change_dir;
        //     break;
        case 'e':case 'E':
            char* parts[2] = {current_dir,dir_content[scroll_index + cursor_index]};
            Open_File_Edit_Popup(Concat(parts,2,'/'));
            goto change_dir;
            
            
        case 'C':
        case 'c':
            if (GET_KEYBOARD_MOD_FLAG(ALT_PRESSED))
            {
                goto cleanup;
            }
            break;
        default:
            break;
        }

        Draw_Rect((Vector2){12, 12}, 295, 100,0x9);// clear dir display area
        for(int i = 0;(i < MAX_DISPLAYED_ENTRIES  && scroll_index + i < dir_content_size );i++){
            uint8_t text_color = i == cursor_index? 0x25 : 0x3F; //is it selected?
            draw_bitmap_string(dir_content[scroll_index + i], 13, 13 + (i*6), 4, 6, text_color, NULL, true, 1);
        }
        input = getc();
    }
cleanup:
    free(current_dir);

    if(dir_content) {
        for(int i = 0; i < dir_content_size; i++) {
            free(dir_content[i]);
        }
        free(dir_content);
    }
    for (int i = 0; i < 16/*  * 512 */; i++) {
        fb[i] = rand() % 256;  
    }
    sleep(1000);
}
