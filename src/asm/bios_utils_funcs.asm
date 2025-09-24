global read_usb_sector_asm;(BYTE* buff, LBA_t sector, UINT count)
global write_usb_sector_asm  ; (BYTE* buff, LBA_t sector, UINT count)
global get_usb_drive_params_asm  ; ()
global test_16func ; ()


%define ARGS_ADDR  0x1000   ; LBA(4) + count(1) stored here
%define OUTPUT_ADDR   0x1010   ; Disk Address Packet buffer location


;========================================================
; NO RET/RETF at END!!(end stub being after the loaded func)
;========================================================

read_usb_sector_asm:
    mov ax, 0
    mov ds, ax            ; assume segment 0 for simplicity

    mov si, ARGS_ADDR
    mov ax, [si]          ; load low word LBA
    mov word [OUTPUT_ADDR + 8], ax
    mov ax, [si + 2]      ; load high word LBA
    mov word [OUTPUT_ADDR + 10], ax

    mov al, [si + 4]      ; load count (byte)
    mov ah, 0
    mov word [OUTPUT_ADDR + 2], ax

    mov byte [OUTPUT_ADDR], 0x10
    mov byte [OUTPUT_ADDR + 1], 0

    mov word [OUTPUT_ADDR + 4], ARGS_ADDR  ; buffer offset
    mov word [OUTPUT_ADDR + 6], 0           ; buffer segment (0 assumed)

    mov dl, 0x81            ; drive number

    mov si, OUTPUT_ADDR
    mov ah, 0x42            ; extended read
    int 0x13
    jc read_error

    mov ax, 0

read_error:
    mov ax, 1

;-------------------------------------------
write_usb_sector_asm:
    mov ax, 0
    mov ds, ax            ; assume segment 0 for simplicity

    mov si, ARGS_ADDR
    mov ax, [si]          ; load low word LBA
    mov word [OUTPUT_ADDR + 8], ax
    mov ax, [si + 2]      ; load high word LBA
    mov word [OUTPUT_ADDR + 10], ax

    mov al, [si + 4]      ; load count (byte)
    mov ah, 0
    mov word [OUTPUT_ADDR + 2], ax

    mov byte [OUTPUT_ADDR], 0x10
    mov byte [OUTPUT_ADDR + 1], 0

    mov word [OUTPUT_ADDR + 4], ARGS_ADDR  ; buffer offset
    mov word [OUTPUT_ADDR + 6], 0           ; buffer segment (0 assumed)

    mov dl, 0x81            ; drive number

    mov si, OUTPUT_ADDR
    mov ah, 0x43            ; extended write
    int 0x13
    jc write_error

    mov ax, 0

write_error:
    mov ax, 1
;-------------------------------------------
get_usb_drive_params_asm:
    xor ax, ax
    mov ds, ax              

    ; Set size of buffer in first 2 bytes
    mov si, OUTPUT_ADDR
    mov byte [si], 0x30     ; Size = 30 bytes
    mov byte [si + 1], 0x00

    mov dl, 0x81            ; Drive number (first hard drive)
    mov ah, 0x48            ; Function: Get Drive Parameters
    int 0x13
    jc drive_param_error

    xor ax, ax              ; Success
    jmp get_usb_drive_params_asm_end

drive_param_error:
    mov ax, 1
get_usb_drive_params_asm_end:
[BITS 16]
test_16func:
    mov word [OUTPUT_ADDR], 'b'
    mov word [OUTPUT_ADDR+2], 0 
    
    sti
    mov ah, 0x0E        ; BIOS teletype output
    mov al, 'A'         ; Character to print
    int 0x10            ; Call BIOS to print 'A'

