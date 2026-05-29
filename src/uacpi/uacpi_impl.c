#include "arch/x86/paging/arch_paging.h"
#include "locks.h"
#include "scheduler/scheduler.h"
#include "uacpi_impl_defs.h"

#include "uacpi/kernel_api.h"

#include "asm-defs/asm.h"

#include "uacpi/types.h"


#include "memory.h"

#include "pci/pci_config_io.h"


#include "paging/arch_paging.h"

#include "kernel_api.h"
#include "time.h"

#include "bootloader.h"
#include "Logger.h"
#include "paging.h"
#include "status.h"
#include <stdbool.h>
#include <stdint.h>






void uacpi_kernel_log(uacpi_log_level level, const uacpi_char* msg){
    char* msg_type[] = {
        "UACPI_LOG_ERROR",
        "UACPI_LOG_WARN",
        "UACPI_LOG_INFO",
        "UACPI_LOG_TRACE",
        "UACPI_LOG_DEBUG",
    };
    Sys_log("[%s] %s",msg_type[level],msg);
}

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address){
    *(uintptr_t*)out_rsdp_address = (uintptr_t)_rsdp_ptr;
    return UACPI_STATUS_OK;
}

void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len){
    page_index page = addr & ~(PAGE_SIZE-1);
    uintptr_t offset = addr & (PAGE_SIZE-1);
    page_index page_amount = ((offset+len + ((PAGE_SIZE/2) -1)))/PAGE_SIZE;
    
    page_index index = k_append_pages(page, page_amount, true, false);
    
    return (void*)(index? index + offset : 0);
}

void uacpi_kernel_unmap(void *addr, uacpi_size len){
    
    page_index page = (uintptr_t)addr & ~(PAGE_SIZE-1);
    uintptr_t offset = (uintptr_t)addr & (PAGE_SIZE-1);
    page_index page_amount = ((offset+len + ((PAGE_SIZE/2) -1)))/PAGE_SIZE;
    
    for (page_index i = 0; i < page_amount; i++){
        unmap_page(page + i);
    }
}


#ifndef UACPI_BAREBONES_MODE













/*
*
* PCI
*
*/
uacpi_status uacpi_kernel_pci_device_open(
    uacpi_pci_address address, uacpi_handle *out_handle
) {
    struct uacpi_pci_device_handle* handle = kmalloc(sizeof(struct uacpi_pci_device_handle));
    if (!handle) return UACPI_STATUS_OUT_OF_MEMORY;

    handle->segment  = address.segment;
    handle->bus      = address.bus;
    handle->device   = address.device;
    handle->function = address.function;

    *out_handle = (uacpi_handle)handle;
    return UACPI_STATUS_OK;
}

void uacpi_kernel_pci_device_close(uacpi_handle handle){
    kfree((void*)handle);
}

uacpi_status uacpi_kernel_pci_read8(
    uacpi_handle device, uacpi_size offset, uacpi_u8 *value
) {
    struct uacpi_pci_device_handle *handle = (struct uacpi_pci_device_handle *)device;
    *value = pci_config_read8(handle->bus, handle->device, handle->function, offset);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_read16(
    uacpi_handle device, uacpi_size offset, uacpi_u16 *value
) {
    struct uacpi_pci_device_handle *handle = (struct uacpi_pci_device_handle *)device;
    *value = pci_config_read16(handle->bus, handle->device, handle->function, offset);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_read32(
    uacpi_handle device, uacpi_size offset, uacpi_u32 *value
) {
    struct uacpi_pci_device_handle *handle = (struct uacpi_pci_device_handle *)device;
    *value = pci_config_read32(handle->bus, handle->device, handle->function, offset);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write8(
    uacpi_handle device, uacpi_size offset, uacpi_u8 value
) {
    struct uacpi_pci_device_handle *handle = (struct uacpi_pci_device_handle *)device;
    pci_config_write8(handle->bus, handle->device, handle->function, offset, value);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write16(
    uacpi_handle device, uacpi_size offset, uacpi_u16 value
) {
    struct uacpi_pci_device_handle *handle = (struct uacpi_pci_device_handle *)device;
    pci_config_write16(handle->bus, handle->device, handle->function, offset, value);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write32(
    uacpi_handle device, uacpi_size offset, uacpi_u32 value
) {
    struct uacpi_pci_device_handle *handle = (struct uacpi_pci_device_handle *)device;
    pci_config_write32(handle->bus, handle->device, handle->function, offset, value);
    return UACPI_STATUS_OK;
}










/*
*
* IO
*
*/
uacpi_status uacpi_kernel_io_map(
    uacpi_io_addr base,
    uacpi_size len,
    uacpi_handle *out_handle
) {
    struct uacpi_io_region_handle *r = (struct uacpi_io_region_handle *)kmalloc(sizeof(struct uacpi_io_region_handle));
    if (!r)
        return UACPI_STATUS_OUT_OF_MEMORY;

    r->base = base;
    r->len  = len;

    *out_handle = (uacpi_handle)r;
    return UACPI_STATUS_OK;
}

void uacpi_kernel_io_unmap(uacpi_handle handle) {
    kfree(handle);
}

/* READS */
uacpi_status uacpi_kernel_io_read8(
    uacpi_handle h,
    uacpi_size offset,
    uacpi_u8 *out_value
) {
    struct uacpi_io_region_handle *r = (struct uacpi_io_region_handle *)h;

    if (offset >= r->len)
        return UACPI_STATUS_INVALID_ARGUMENT;

    *out_value = inb((uint16_t)(r->base + offset));
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_read16(
    uacpi_handle h,
    uacpi_size offset,
    uacpi_u16 *out_value
) {
    struct uacpi_io_region_handle *r = (struct uacpi_io_region_handle *)h;

    if (offset + 1 >= r->len)
        return UACPI_STATUS_INVALID_ARGUMENT;

    *out_value = inw((uint16_t)(r->base + offset));
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_read32(
    uacpi_handle h,
    uacpi_size offset,
    uacpi_u32 *out_value
) {
    struct uacpi_io_region_handle *r = (struct uacpi_io_region_handle *)h;

    if (offset + 3 >= r->len)
        return UACPI_STATUS_INVALID_ARGUMENT;

    *out_value = inl((uint16_t)(r->base + offset));
    return UACPI_STATUS_OK;
}

/* WRITES */
uacpi_status uacpi_kernel_io_write8(
    uacpi_handle h,
    uacpi_size offset,
    uacpi_u8 value
) {
    struct uacpi_io_region_handle *r = (struct uacpi_io_region_handle *)h;

    if (offset >= r->len)
        return UACPI_STATUS_INVALID_ARGUMENT;

    outb((uint16_t)(r->base + offset), value);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write16(
    uacpi_handle h,
    uacpi_size offset,
    uacpi_u16 value
) {
    struct uacpi_io_region_handle *r = (struct uacpi_io_region_handle *)h;

    if (offset + 1 >= r->len)
        return UACPI_STATUS_INVALID_ARGUMENT;

    outw((uint16_t)(r->base + offset), value);
    return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write32(
    uacpi_handle h,
    uacpi_size offset,
    uacpi_u32 value
) {
    struct uacpi_io_region_handle *r = (struct uacpi_io_region_handle *)h;

    if (offset + 3 >= r->len)
        return UACPI_STATUS_INVALID_ARGUMENT;

    outl((uint16_t)(r->base + offset), value);
    return UACPI_STATUS_OK;
}


/*
*
* Alloc
*
*/
void *uacpi_kernel_alloc(uacpi_size size) {
    return kmalloc(size);
}

void uacpi_kernel_free(void *mem){
    kfree(mem);
}
#endif // !UACPI_BAREBONES_MODE