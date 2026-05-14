#pragma once

#include <stdint.h>


//bullshit code just for stuff to work
void acquire_lock(uint32_t *addr, uint32_t bit);
int try_acquire_lock(uint32_t *addr, uint32_t bit);

void release_lock(uint32_t *addr, uint32_t bit);