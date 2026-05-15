#pragma once
#include <stdint.h>

void acquire_lock(uintptr_t addr);
void release_lock(uintptr_t addr);