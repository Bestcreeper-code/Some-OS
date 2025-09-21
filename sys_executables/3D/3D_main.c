#include "../../src/headers/io.h"
#include "../../src/headers/video.h"
#include "../../src/headers/vga_modes.h"
#include "../../src/headers/memory.h"
#include "../../src/headers/time.h"
#include "../../src/headers/math.h"
#include "Graphics/graphics.h"
#include "res.h"
//y capped to 125 smh

Vector2 pl_pos = {0,0};
int rotation = 90;

void app_main() {
    uint8_t* second_buffer = malloc(SCREEN_PITCH * SCREEN_HEIGHT);
    if(!second_buffer){
        printf("[3D] memory allocation failure");
        return;
    }

    vga_set_mode(0x13);
    clear_13h_screen(0x0);
    
    Wall walls[2] = {
        {.point1=(Vector2){20,10},.point2=(Vector2){-20,10},.color=4},
        {.point1=(Vector2){-20,10},.point2=(Vector2){-20,-10},.color=3},
    };

    while (true)
    {
        memset((void*)second_buffer, 0x0, SCREEN_PITCH * SCREEN_HEIGHT);//clear

        unsigned char input = getc_nb();
        if (input == 's') pl_pos.y--;
        if (input == 'w') pl_pos.y++;
        if (input == 'd') rotation = (int)(rotation+3)%360;
        if (input == 'a') rotation = (int)(rotation-3)%360;
        Draw_Screen(pl_pos, DEG_TO_RAD(rotation), walls, 2, second_buffer);  // face up

        char str[16];
        intToStr((int)pl_pos.x, str);

        memcpy((void*)graph_mode_fb,second_buffer,SCREEN_PITCH * SCREEN_HEIGHT);
        
        int string_len =16;

        for (int i = 0; i < string_len; i++) {
            draw_bitmap_char(str[i], 10 + i * 8, 10, 8, 8, 0x0F, NULL, true, false,false);
        }//bug only half renders bc too fast and wall doesnt change

        sleep(16);
    }
    free(second_buffer);
}
