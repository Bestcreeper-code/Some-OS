#ifndef RES_H
#define RES_H

#include <stdint.h>
#include "../../src/headers/video.h"

#define PAINT_FILE_MAGIC "PAINT\0"

typedef struct {
    uint8_t magic[6];//PAINT\0 
    uint16_t width;
    uint16_t height;
    uintptr_t data_start;
    RGBColor colors[];    
}  __attribute__((__packed__)) PaintFileHeader;


char* intToStr(int num);
char* String_Input_Popup(int x, int y,int width);
uint8_t Open_File_Edit_Popup(char* file);

void* create_default_paint_header();
#endif // RES_H
