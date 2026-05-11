[BITS 32]
global _start

extern kmain
extern bootloader_asm_entry

KERNEL_VMA  equ 0xC0000000
KERNEL_PHYS equ 0x00100000

section .data
align 4096


boot_page_dir:
   
    dd 0x00000083
    times (768 - 1) dd 0

    ; Map N entries starting at PDE 768
    %assign i 0
    %assign N 64              ; number of 4MB pages to map (example: 256MB)

    %rep N
        dd (i * 0x00400000) | 0x00000083
        %assign i i + 1
    %endrep
    
    times (1024 - 768 - N) dd 0

section .bss
align 16
resb 32768                      ; 32KB bootstrap stack
boot_stack_top:

section .boot
_start:
    ; eax = multiboot magic, ebx = multiboot info p_ptr 
    mov edi, eax
    mov esi, ebx

    ; Load physical address of page directory
    mov ecx, (boot_page_dir - KERNEL_VMA)
    mov cr3, ecx

    ; Enable PSE in CR4
    mov ecx, cr4
    or  ecx, 0x00000010
    mov cr4, ecx

    ;Enable paging 
    mov ecx, cr0
    or  ecx, 0x80000000
    mov cr0, ecx

    ; Far jump to higher half
    lea ecx, [higher_half]
    jmp ecx
section .text 
higher_half:
    ; Set up a real stack 
    mov esp, boot_stack_top

    ; Call bootloader parser while identity map is still live
    ; (multiboot struct is at a low physical address)
    push esi    ; multiboot info ptr
    push edi    ; magic
    call bootloader_asm_entry
    add esp, 8

    ; Remove identity map (0)
    mov dword [boot_page_dir], 0
    invlpg [0]

    call kmain

    cli
    hlt
.hang:
    jmp .hang