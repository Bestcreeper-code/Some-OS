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

char _kernel_end = 0; 
char _kernel_start = 0; 

void main(int argc, char** argv) {
    Sys_log("YEYEYEYEYYEY");
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

    draw_bitmap_string(current_dir,0,0,4,6,0x12,NULL,true,false,1);

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

            char* conc_fullpath =Concat(parts,2,'/');
            Open_File_Edit_Popup(conc_fullpath);
            
            free(conc_fullpath);

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
            char* curr_file_name = strdup(dir_content[scroll_index + i]);
            if(!curr_file_name)continue;
            uint16_t color = get_file_color(curr_file_name);
            if(color>255)color =0X7;
            if(curr_file_name[strlen(curr_file_name)-1] =='/')color = 0X16;
            
            if(i == cursor_index){
                color = 0x25;
                uint8_t curr_file_n_len = strlen(curr_file_name);
                char* temp_file_name = malloc(curr_file_n_len+3);//\0 + space and <
                if(temp_file_name){
                    strcpy(temp_file_name, curr_file_name);
                    temp_file_name[curr_file_n_len] = ' ';
                    temp_file_name[curr_file_n_len+1] = '<';
                    temp_file_name[curr_file_n_len+2] = '\0';
                    free(curr_file_name);
                    curr_file_name = temp_file_name;
                }
            }
            draw_bitmap_string(curr_file_name, 13, 13 + (i*6), 4, 6, color, NULL, true, false,1);
            free(curr_file_name);
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
        graph_mode_fb[i] = rand() % 256;  
    }
    sleep(1000);
}
