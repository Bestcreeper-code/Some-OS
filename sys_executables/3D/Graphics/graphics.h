#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdint.h>
typedef struct 
{
    float x,y;
} Vector2;



void Draw_Quad(const Vector2* verts, char color, uint8_t* buffer);
void Draw_Rect(Vector2 pos, int width, int height,char color, uint8_t* buffer);
#endif // GRAPHICS_H
