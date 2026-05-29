OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

KERNEL_PHYS = 0x00100000;
KERNEL_VMA  = 0xFFFFFFFF80000000;

SECTIONS
{
    . = KERNEL_PHYS;
    .boot : {
        *(.boot)
        *(.boot*)
    }
    _boot_phys_end = .;

    /* Higher-half kernel */
    . = (KERNEL_VMA + _boot_phys_end);

    _kernel_start = .;

    .text   : AT(ADDR(.text)   - KERNEL_VMA) { *(.text*)   }
    .rodata : AT(ADDR(.rodata) - KERNEL_VMA) { *(.rodata*) }
    .data   : AT(ADDR(.data)   - KERNEL_VMA) {
        *(.data*)

        /* drivers */
        __start_core_drivers = .; *(.core_drivers) __stop_core_drivers = .;
        __start_dev_drivers  = .; *(.dev_drivers)  __stop_dev_drivers  = .;
        __start_fs_drivers   = .; *(.fs_drivers)   __stop_fs_drivers   = .;
        __start_late_drivers = .; *(.late_drivers)  __stop_late_drivers = .;
    }
    
    .bss    : AT(ADDR(.bss)    - KERNEL_VMA) { *(.bss*) *(COMMON) }

    _kernel_end = .;
}