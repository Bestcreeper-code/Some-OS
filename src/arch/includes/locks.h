#pragma once

#include <stdint.h>


//bullshit code just for stuff to work
void acquire_lock(uintptr_t *addr, uint8_t bit);
int try_acquire_lock(uintptr_t *addr, uint8_t bit);

void release_lock(uintptr_t *addr, uint8_t bit);