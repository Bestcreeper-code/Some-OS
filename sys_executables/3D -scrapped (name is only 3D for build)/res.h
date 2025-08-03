#ifndef RES_H
#define RES_H
#include "Graphics/graphics.h"

#define RAY_MAX_STEPS           50.0f
#define RAY_STEP_SPEED_MULT     1

#define RENDER_DISTANCE         70.0f

#define SCREEN_WIDTH            320
#define SCREEN_PITCH            512
#define SCREEN_HEIGHT           125



typedef struct 
{
    Vector2 point1;
    Vector2 point2;
    char color;
} Wall;


void intToStr(int num, char* str);

double Raycast(const Vector2 start_pos, float angle, Wall* walls, int wall_amount, char* color);
void Draw_Screen(const Vector2 pos, float angle, Wall* walls, int wall_amount, void* buffer);
#endif // RES_H
