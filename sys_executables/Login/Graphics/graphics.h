#ifndef GRAPHICS_H
#define GRAPHICS_H

typedef struct 
{
    int x,y;
} Vector2;



void Draw_Quad(const Vector2* verts, char color);
void Draw_Rect(Vector2 pos, int width, int height,char color);
#endif // GRAPHICS_H
