#include "drivers.h"
#include "Logger.h"

extern init_driver_t __start_core_drivers[], __stop_core_drivers[];
extern init_driver_t __start_dev_drivers[], __stop_dev_drivers[];
extern init_driver_t __start_fs_drivers[], __stop_fs_drivers[];
extern init_driver_t __start_late_drivers[], __stop_late_drivers[];




int init_drivers(init_driver_t *start, init_driver_t *stop) {
    for (init_driver_t *drv = start; drv < stop; drv++) {
        Sys_color_log_NoPos("   --------- initing %s\n", ANSI_YELLOW, ANSI_BG_BLACK, drv->name);
        drv->init();
    }
    return 0 ;
}

 