#include "../../src/headers/io.h"
#include "../../src/headers/console.h"
#include "../../src/headers/random.h"
#include "../../src/headers/memory.h"
#include "../../src/headers/time.h"
#include "res.h"

const int move_delay_limit = 25;

void app_main() {
    pl_parts = malloc(sizeof(Vector2) * 2);
    pl_parts[0].x = 40;
    pl_parts[0].y = 12;
    pl_parts[1].x = 40;
    pl_parts[1].y = 13;
    ClearScreen();
    Draw_Border();  // draw once at start
    unsigned char c = 0;
    int move_delay_counter = 0;
    
    while (true) {
        // --- Handle input ---
        c = GetInputCharNonBlocking();

        if (c == KEY_UP && dir_y != 1 && pl_parts[0].y > topleft.y + 1) {
            dir_y = -1;
            dir_x = 0;
        } else if (c == KEY_DOWN && dir_y != -1 && pl_parts[0].y < bottomright.y - 1) {
            dir_y = 1;
            dir_x = 0;
        } else if (c == KEY_LEFT && dir_x != 1 && pl_parts[0].x > topleft.x + 1) {
            dir_y = 0;
            dir_x = -1;
        } else if (c == KEY_RIGHT && dir_x != -1 && pl_parts[0].x < bottomright.x - 1) {
            dir_y = 0;
            dir_x = 1;
        } else if (c == 'q') {
            char next_x = pl_parts[pl_size - 1].x - pl_parts[pl_size - 2].x;
            char next_y = pl_parts[pl_size - 1].y - pl_parts[pl_size - 2].y;
            pl_parts = realloc(pl_parts, sizeof(Vector2) * (pl_size + 1));
            if (!pl_parts) {
                printf("Memory allocation failed!\n");
                return;
            }
            pl_parts[pl_size].x = pl_parts[pl_size - 1].x + next_x;
            pl_parts[pl_size].y = pl_parts[pl_size - 1].y + next_y;
            pl_size++;
        }

        // --- Movement delay logic ---
        move_delay_counter++;
        if (move_delay_counter >= move_delay_limit) {
            move_delay_counter = 0;

            // Clear old position
            for (int i = 0; i < pl_size; i++) {
                put_char(pl_parts[i].x, pl_parts[i].y, ' ', 0x0F);
            }

            for (int i = pl_size - 1; i > 0; i--) {
                pl_parts[i].x = pl_parts[i - 1].x;
                pl_parts[i].y = pl_parts[i - 1].y;
            }

            pl_parts[0].x += dir_x;
            pl_parts[0].y += dir_y;

            put_char(pl_parts[0].x, pl_parts[0].y, 0x08, 0x04); // head
            for (int i = 1; i < pl_size; i++) {
                put_char(pl_parts[i].x, pl_parts[i].y, 0xDB, 0x0A);
                if (pl_parts[0].x == pl_parts[i].x && pl_parts[0].y == pl_parts[i].y) {
                    ClearScreen();
                    move_cursor(30, 12);
                    printf("Game Over! You collided with yourself.\n");
                    goto end;
                }
            }
        }

        c = 0;
        sleep(10);  // 10ms per loop
    }

end:
    free(pl_parts);
}
