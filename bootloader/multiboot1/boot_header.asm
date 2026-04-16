[BITS 32]
section .text
    align 4
    dd 0x1BADB002
    dd 0x07
    dd -(0x1BADB002 + 0x07)

dd 0
dd 0
dd 0
dd 0
dd 0

dd 0
dd 1024
dd 768
dd 32

; struct multiboot_header
; {
;   /* Must be MULTIBOOT_MAGIC - see above. */
;   uint32_t magic;

;   /* Feature flags. */
;   uint32_t flags;

;   /* The above fields plus this one must equal 0 mod 2^32. */
;   uint32_t checksum;

;   /* These are only valid if MULTIBOOT_AOUT_KLUDGE is set. */
;   uint32_t header_addr;
;   uint32_t load_addr;
;   uint32_t load_end_addr;
;   uint32_t bss_end_addr;
;   uint32_t entry_addr;

;   /* These are only valid if MULTIBOOT_VIDEO_MODE is set. */
;   uint32_t mode_type;
;   uint32_t width;
;   uint32_t height;
;   uint32_t depth;
; };