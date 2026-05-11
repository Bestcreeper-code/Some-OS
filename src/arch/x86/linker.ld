OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

KERNEL_VMA = 0xC0000000;
PHYS_BASE  = 0x00100000;

SECTIONS
{
    /* --- boot stub: VMA == LMA == physical, no paging yet --- */
    . = PHYS_BASE;

    .boot : {
        *(.boot)          /* entry.asm */
    }

    _boot_phys_end = .;   /* physical address right after boot stub */

    /*higher half*/
    . = KERNEL_VMA + _boot_phys_end;

    _kernel_start = .;


    .text   : AT(ADDR(.text)   - KERNEL_VMA) { *(.text*)   }
    .rodata : AT(ADDR(.rodata) - KERNEL_VMA) { *(.rodata*) }
    .data   : AT(ADDR(.data)   - KERNEL_VMA) { *(.data*)   }
    .bss    : AT(ADDR(.bss)    - KERNEL_VMA) { *(.bss*)    }

    _kernel_end = .;      /* virtual address */
}