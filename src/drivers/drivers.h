#pragma once
#include "compiler_defs.h"

typedef struct {
    const char *name;
    int (*init)(void);
} init_driver_t;


#define REGISTER_DRIVER(d_name, init_f) \
    GCC_ATTR((section("k_drivers"), used)) init_driver_t __##d_name##_driver_struct = {.name= #d_name, .init=init_f}


void drivers_init();