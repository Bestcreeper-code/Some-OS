#include "../../src/headers/io.h"
#include "../../src/headers/console.h"
#include "../../src/headers/random.h"
#include "../../src/headers/memory.h"
#include "../../src/headers/time.h"
#include "res.h"

const int move_delay_limit = 25;

Vector2* pl_parts = NULL;
int pl_size = 2;

int dir_x = 0;
int dir_y = -1;

extern Vector2 topleft;
extern Vector2 bottomright;
void Draw_Border();

int app_main() {
    pl_parts = malloc(sizeof(Vector2) * pl_size);
    if (!pl_parts) {
        printf("Initial allocation failed!\n");
        return;
    }

    // Initial snake segments
    pl_parts[0].x = 30;
    pl_parts[0].y = 7;
    pl_parts[1].x = 30;
    pl_parts[1].y = 8;

    ClearScreen();
    Draw_Border();

    unsigned char c = 0;
    int move_delay_counter = 0;

    while (true) {
        // --- Input ---
        c = GetInputCharNonBlocking();
        if (c == KEY_UP && dir_y != 1) { dir_x = 0; dir_y = -1; }
        else if (c == KEY_DOWN && dir_y != -1) { dir_x = 0; dir_y = 1; }
        else if (c == KEY_LEFT && dir_x != 1) { dir_x = -1; dir_y = 0; }
        else if (c == KEY_RIGHT && dir_x != -1) { dir_x = 1; dir_y = 0; }

        // Grow snake on 'q'
        else if (c == 'q') {
            int dx = pl_parts[pl_size - 1].x - pl_parts[pl_size - 2].x;
            int dy = pl_parts[pl_size - 1].y - pl_parts[pl_size - 2].y;

            Vector2* temp = realloc(pl_parts, sizeof(Vector2) * (pl_size + 1));
            if (!temp) {
                printf("Memory allocation failed!\n");
                free(pl_parts);
                return -1;
            }
            pl_parts = temp;
            pl_parts[pl_size].x = pl_parts[pl_size - 1].x + dx;
            pl_parts[pl_size].y = pl_parts[pl_size - 1].y + dy;
            pl_size++;
        }

        // --- Movement ---
        move_delay_counter++;
        if (move_delay_counter >= move_delay_limit) {
            move_delay_counter = 0;

            // Clear old position
            for (int i = 0; i < pl_size; i++) {
                put_char(pl_parts[i].x, pl_parts[i].y, ' ', 0x0F);
            }

            // Shift body
            for (int i = pl_size - 1; i > 0; i--) {
                pl_parts[i] = pl_parts[i - 1];
            }

            // Move head
            pl_parts[0].x += dir_x;
            pl_parts[0].y += dir_y;

            // Check collision with walls
            if (pl_parts[0].x <= topleft.x || pl_parts[0].x >= bottomright.x ||
                pl_parts[0].y <= topleft.y || pl_parts[0].y >= bottomright.y) {
                ClearScreen();
                move_cursor(30, 12);
                printf("Game Over! You hit the wall.\n");
                break;
            }

            // Check self-collision
            for (int i = 1; i < pl_size; i++) {
                if (pl_parts[0].x == pl_parts[i].x && pl_parts[0].y == pl_parts[i].y) {
                    ClearScreen();
                    move_cursor(30, 12);
                    printf("Game Over! You collided with yourself.\n");
                    goto end;
                }
            }

            // Draw new position
            put_char(pl_parts[0].x, pl_parts[0].y, 0x08, 0x04); // head
            for (int i = 1; i < pl_size; i++) {
                put_char(pl_parts[i].x, pl_parts[i].y, 0xDB, 0x0A); // body
            }
        }

        sleep(10);  // Sleep for 10ms
    }

end:
    free(pl_parts);
    return 0;
}
