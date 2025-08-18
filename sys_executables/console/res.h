#ifndef RES_H
#define RES_H

#include <stdbool.h>
#include "../../src/headers/console.h"

extern char* command_History[MAX_HISTORY];
extern int command_History_count;
static char* currpath ;

void Start_Console();
char* Console_Get_Command();
bool Console_Process_Command(char* command);
char** Split(const char* str, char delimiter, int max_tokens, int* out_count);
void EndSplit(char** tokens, int count);
#endif // RES_H
