extern _start

%define MULTIBOOT2_HEADER_MAGIC 0xE85250D6
%define GRUB_MULTIBOOT_ARCHITECTURE_I386 0


%define MULTIBOOT_HEADER_TAG_END  0
%define MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST  1
%define MULTIBOOT_HEADER_TAG_ADDRESS  2
%define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS  3
%define MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS  4
%define MULTIBOOT_HEADER_TAG_FRAMEBUFFER  5
%define MULTIBOOT_HEADER_TAG_MODULE_ALIGN  6
%define MULTIBOOT_HEADER_TAG_EFI_BS        7
%define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI32  8
%define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI64  9
%define MULTIBOOT_HEADER_TAG_RELOCATABLE  10

%define MULTIBOOT_HEADER_TAG_OPTIONAL 1

[BITS 32]
section .text
    align 8 ;align to 64 bits
multiboot_header_start:
    dd MULTIBOOT2_HEADER_MAGIC   ;magic
    dd 0x0          
    dd (multiboot_header_end - multiboot_header_start)
    dd -(MULTIBOOT2_HEADER_MAGIC + GRUB_MULTIBOOT_ARCHITECTURE_I386 + (multiboot_header_end - multiboot_header_start))
; ;Multiboot2 information request
;     dw MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST
;     dw 0
;     dd (mb_tags_end - mb_tags_start +8)
; mb_tags_start:
; mb_tags_end:
;entry address tag of Multiboot2 header
    align 8
    entry_address_tag_start:        
        dw MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS
        dw 0
        dd entry_address_tag_end - entry_address_tag_start
        ;  entry_addr 
        dd _start
    entry_address_tag_end:
    
    align 8
    framebuffer_tag_start:  
        dw MULTIBOOT_HEADER_TAG_FRAMEBUFFER
        dw 0
        dd framebuffer_tag_end - framebuffer_tag_start
        dd 1024
        dd 768
        dd 32
    framebuffer_tag_end:
    
    align 8
    ;end tag
        dw MULTIBOOT_HEADER_TAG_END
        dw 0
        dd 8
multiboot_header_end: