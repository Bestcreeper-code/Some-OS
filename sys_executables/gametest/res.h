#ifndef RES_H
#define RES_H

typedef struct {
    char x;
    char y;
} Vector2;

extern Vector2 topleft;
extern Vector2 bottomright;

extern int pl_size;

void Draw_Border();

#endif // RES_H