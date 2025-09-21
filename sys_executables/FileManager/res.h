#ifndef RES_H
#define RES_H

#include <stdint.h>

typedef struct 
{
    char* key;
    uint8_t val;
} __attribute__((packed)) Vector_str_chr;


void intToStr(int num, char* str);
char* String_Input_Popup(int x, int y,int width);
uint8_t Open_File_Edit_Popup(char* file);

uint16_t get_file_color(const char* name);
#endif // RES_H
