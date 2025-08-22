#ifndef RES_H
#define RES_H

#include <stdint.h>

char* intToStr(int num);
char* String_Input_Popup(int x, int y,int width);
uint8_t Open_File_Edit_Popup(char* file);

char* xor_crypt(const char* value, int value_size, const char* key, int key_size);

#endif // RES_H
