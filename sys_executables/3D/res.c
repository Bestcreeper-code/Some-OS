#include "res.h"
#include "../../src/headers/math.h"
#include <stdbool.h>


void intToStr(int num, char* str) {
    int i = 0;
    bool is_negative = false;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0) {
        is_negative = true;
        num = -num;
    }

    while (num > 0) {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }

    if (is_negative) str[i++] = '-';
    str[i] = '\0';

    for (int j = 0; j < i / 2; j++) {
        char tmp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = tmp;
    }
}





int cross(Vector2 a, Vector2 b) {
    return a.x*b.y - a.y*b.x;
}

int orient(Vector2 a, Vector2 b, Vector2 c) {
    return cross((Vector2){b.x - a.x, b.y - a.y}, (Vector2){c.x - a.x, c.y - a.y});
}

bool intersects(Vector2 a, Vector2 b, Vector2 c, Vector2 d) {
    int oa = orient(c,d,a), 
        ob = orient(c,d,b),            
        oc = orient(a,b,c),            
        od = orient(a,b,d);      
    // Proper intersection exists if opposite signs  
    return (oa*ob < 0 && oc*od < 0);
} 




double Raycast(const Vector2 start_pos, float angle, Wall* walls, int wall_amount, char* color){
    Vector2 dir = {.x = cos(angle)*RAY_STEP_SPEED_MULT, .y = sin(angle)*RAY_STEP_SPEED_MULT};
    Vector2 curr_pos = start_pos;
    Vector2 last_pos;
    int steps = 0;

    while (steps < RAY_MAX_STEPS)
    {
        last_pos.x = curr_pos.x;
        last_pos.y = curr_pos.y;

        curr_pos.x += dir.x;
        curr_pos.y += dir.y;

        for (int i = 0;i < wall_amount;i++){
            if(intersects(last_pos, curr_pos, walls[i].point1, walls[i].point2)){
                double dx = curr_pos.x - start_pos.x;
                double dy = curr_pos.y - start_pos.y;
                *color = walls[i].color;
                return sqrt(dx*dx + dy*dy);
            }
        }

        steps++;
    }
    return -1;
}   



void Draw_Screen(const Vector2 pos, float angle, Wall* walls, int wall_amount, void* buffer) {
    const float fov = (60.0f * M_PI) / 180.0f;  // 60 degrees
    float start_angle = angle - (fov / 2.0f);

    for (int i = 0; i < SCREEN_WIDTH; i++) {
        float ray_angle = start_angle + ((float)i / SCREEN_WIDTH) * fov;

        char color = 0;
        float dist = Raycast(pos, ray_angle, walls, wall_amount, &color);

        if (dist == -1 || dist>100) continue;  // Skip this column if no wall was hit

        dist *= cos(ray_angle - angle);  // fisheye correction

        float coll_size = (RENDER_DISTANCE - dist) / RENDER_DISTANCE * (SCREEN_HEIGHT / 2.0f);//get the size to render on the collumn by multiplying by half the max y 
        int column_height = (int)(coll_size * 2.0f);
        if (column_height > SCREEN_HEIGHT) column_height = SCREEN_HEIGHT;

        int y_start = (int)(SCREEN_HEIGHT / 2.0f - coll_size);
        if (y_start < 0) y_start = 0;

        Draw_Rect((Vector2){i, y_start}, 1, column_height, color, buffer);
    }
}
