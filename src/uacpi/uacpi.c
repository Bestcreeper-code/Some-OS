#include "arch_paging.h"

#include "kernel_api.h"

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
    // unmap_page();
}