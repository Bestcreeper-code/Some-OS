#ifndef RES_H
#define RES_H

#include <stdint.h>

char* String_Input_Popup(int x, int y,int width);
uint8_t Open_File_Edit_Popup(char* file);

char** split_text_lines(const char* text,int char_per_line, int* out_line_count);

#endif // RES_H
