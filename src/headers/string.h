#ifndef STRING_H
#define STRING_H

#define STRING_MAX_LEN 2048

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>



char *strchr(const char *str, int c);
size_t strlen(const char* str);
char *strdup(const char *s); 
char *strndup( const char *str, size_t size );
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
int strcmp(const char* s1, const char* s2);



void* memset(void *dest, int value, size_t count);
void* memcpy(void* dest, const void* src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memmove(void *dest, const void *src, size_t n);



bool Starts_With(const char* string,const char* prefix);
// Dont forget to EndSplit() 
char** Split(const char* string, char separator, int max_tokens, int* out_count);
void EndSplit(char** tokens, int count);
char* Concat(char** list, size_t size, char linking_char);
bool InsertChar(char* str, uint32_t pos, char c);
void RemoveChar(char* str, uint32_t index);

//=====conversion functions=====

//str to int
int atoi(const char* str);
//int to str
void itoa(int value, char* str, int base);

//allocates a lowercase copy of <str>
char* strlow(const char* str);
#endif // STRING_H
