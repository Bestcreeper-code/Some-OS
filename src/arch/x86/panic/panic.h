#ifndef CRASHHNDL_H
#define CRASHHNDL_H

#include <stdint.h>

#define MAX_STACK_TRACE_SIZE 16

void _panic_handler(int argc, uint32_t* argv);
void _manual_panic(const char* error, const char* info);

#endif // CRASHHNDL_H