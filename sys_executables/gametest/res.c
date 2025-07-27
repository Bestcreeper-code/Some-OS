#include "res.h"
#include "../../src/headers/io.h"

Vector2 topleft = {25,4};
Vector2 bottomright = {35,14};

int dir_x = 0;
int dir_y = -1;

Vector2* pl_parts;
int pl_size = 1;

void Draw_Border(){
    for (int i = topleft.x; i < bottomright.x; i++) {
        put_char(i, topleft.y, '-', 0x0F); // Top border
        put_char(i, bottomright.y, '-', 0x0F); // Bottom border
    }
    for (int i = topleft.y + 1; i < bottomright.y; i++) {
        put_char(topleft.x, i, '|', 0x0F); // Left border
        put_char(bottomright.x, i, '|', 0x0F); // Right border
    }
    put_char(topleft.x, topleft.y, '+', 0x0F); // Top-left corner
    put_char(bottomright.x, topleft.y, '+', 0x0F); // Top-right corner
    put_char(topleft.x, bottomright.y, '+', 0x0F); // Bottom-left corner
    put_char(bottomright.x, bottomright.y, '+', 0x0F); // Bottom-right corner
}