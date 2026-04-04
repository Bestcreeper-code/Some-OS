#include "drivers.h"
#include "Logger.h"

extern init_driver_t __start_k_drivers[];
extern init_driver_t __stop_k_drivers[];



void drivers_init() {
    for (init_driver_t *drv = __start_k_drivers; drv < __stop_k_drivers; drv++) {
        Sys_log("initializing %s\n", drv->name);
        drv->init();
    }
}