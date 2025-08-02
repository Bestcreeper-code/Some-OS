#include "../../src/headers/io.h"
#include "../../src/headers/video.h"
#include "../../src/headers/memory.h"
#include "../../src/headers/time.h"
#include "Graphics/graphics.h"
//y capped to 125 smh

void app_main() {
    vga_set_mode_13h();
    clear_13h_screen(0x9);
    Draw_Rect((Vector2){30, 10}, 260, 105,0x3F);
    Draw_Rect((Vector2){40, 20}, 240, 85,0x9);
    sleep(100000);
}
