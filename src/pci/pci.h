#pragma once

#include <stdint.h>

uint16_t get_vendor_id(uint8_t bus, uint8_t device, uint8_t function);

int check_all_buses();