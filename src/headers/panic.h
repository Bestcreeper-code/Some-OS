#ifndef CRASHHNDL_H
#define CRASHHNDL_H

#include <stdint.h>

void _panic_handler(int argc, uint32_t* argv);
void _manual_panic(const char* error, const char* info);

#endif // CRASHHNDL_H