#pragma once
#include <stdint.h>

void acquireLock(uintptr_t addr);
void releaseLock(uintptr_t addr);