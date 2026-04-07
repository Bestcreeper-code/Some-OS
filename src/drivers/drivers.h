#pragma once
#include "compiler_defs.h"

typedef struct {
    const char *name;
    int (*init)(void);
} init_driver_t;



#define REGISTER_DRIVER_PHASE(d_name, init_f, phase) \
    GCC_ATTR((section(#phase "_drivers"), used)) \
    init_driver_t __##d_name##_driver_struct = {.name=#d_name, .init=init_f}


#define REGISTER_DRIVER_CORE(d_name, init_f)  REGISTER_DRIVER_PHASE(d_name, init_f, core)
#define REGISTER_DRIVER_DEV(d_name, init_f)   REGISTER_DRIVER_PHASE(d_name, init_f, dev)
#define REGISTER_DRIVER_FS(d_name, init_f)    REGISTER_DRIVER_PHASE(d_name, init_f, fs)
#define REGISTER_DRIVER_LATE(d_name, init_f)  REGISTER_DRIVER_PHASE(d_name, init_f, late)



inline int core_init();
inline int dev_init();
inline int fs_init();
inline int late_init();