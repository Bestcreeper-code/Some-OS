#ifndef MOUSE_H
#define MOUSE_H
#include <stdbool.h>

typedef enum {
    MOUSE_BUTTON_LEFT = 1<<0,
    MOUSE_BUTTON_RIGHT = 1<<1,
    MOUSE_BUTTON_MIDDLE = 1<<2,


    MOUSE_DISPLAYED_FLAG = 1<<7
} Mouse_FLAGS;

void init_mouse();

void mouse_irq_handler(); 

void Redraw_Mouse_Cursor();

void enable_mouse_display();
void disable_mouse_display();

bool Get_Mouse_Button(Mouse_FLAGS button);
void Get_Mouse_Pos(short* x, short* y);

#endif // MOUSE_H
