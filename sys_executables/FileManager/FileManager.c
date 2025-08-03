#include "../../src/headers/io.h"
#include "../../src/headers/video.h"
#include "../../src/headers/memory.h"
#include "../../src/headers/time.h"
#include "Graphics/graphics.h"
//y capped to 125 smh



void app_main() {
    char* current_dir = malloc(16);
    current_dir[0] = '0';
    current_dir[1] = ':';
    current_dir[2] = '/';
    current_dir[3] = '\0';

    vga_set_mode_13h();
    clear_13h_screen(0x9);

    Draw_Rect((Vector2){8, 8}, 303, 108,0x3);
    Draw_Rect((Vector2){12, 12}, 295, 100,0x9);
    draw_bitmap_string("File Manager",0,0,4,6,0x3F,NULL,true,1);
    
    
    sleep(100000);
}
