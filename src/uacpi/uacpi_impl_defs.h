#pragma once


#include "uacpi/types.h"
#include <stdint.h>
struct uacpi_pci_device_handle {
    uint16_t segment;
    uint8_t bus;
    uint8_t device;
    uint8_t function;
};

struct uacpi_io_region_handle{
    uacpi_io_addr base;
    uacpi_size len;
};
