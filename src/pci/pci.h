#pragma once

#include <stdint.h>
uint16_t pci_config_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

uint16_t get_vendor_id(uint8_t bus, uint8_t device, uint8_t function);

int check_all_buses();