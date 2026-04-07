#include "drivers.h"
#include "Logger.h"

extern init_driver_t __start_core_drivers[], __stop_core_drivers[];
extern init_driver_t __start_dev_drivers[], __stop_dev_drivers[];
extern init_driver_t __start_fs_drivers[], __stop_fs_drivers[];
extern init_driver_t __start_late_drivers[], __stop_late_drivers[];




static int init_drivers(init_driver_t *start, init_driver_t *stop) {
    for (init_driver_t *drv = start; drv < stop; drv++) {
        Sys_log("initing %s\n", drv->name);
        drv->init();
    }
}

inline int core_init(){
    return init_drivers(__start_core_drivers, __stop_core_drivers);
}

inline int dev_init(){
    return init_drivers(__start_dev_drivers, __stop_dev_drivers);
}

inline int fs_init(){
    return init_drivers(__start_fs_drivers, __stop_fs_drivers);
}

inline int late_init(){
    return init_drivers(__start_late_drivers, __stop_late_drivers);
}