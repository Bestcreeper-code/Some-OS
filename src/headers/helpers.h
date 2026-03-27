#pragma once 
#include "arch_types.h"
#include <stddef.h>

#define RET_IF( equation, retval) if(equation) return retval

ssize_t bitmap_alloc_first(char *bitmap, size_t nbytes);
ssize_t wbitmap_alloc_first(char *bitmap, size_t nbytes);

void bitmap_free_bit(char *bitmap, size_t pos);

