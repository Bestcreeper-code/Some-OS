; extern mb_struct_ptr
extern bootloader_c_entry

%define MULTIBOOT_STRUCT_SIZE 0x78

global bootloader_asm_entry

section .text


bootloader_asm_entry:
    push ebx ; multiboot_info pointer
    push eax ; magic
    call bootloader_c_entry
    add esp, 8
    
    ret