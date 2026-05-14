global acquire_lock
global release_lock
global try_acquire_lock

; void acquire_lock(uint32_t *addr, uint32_t bit)
acquire_lock:
    mov eax, [esp + 8]     ; bit index
    mov edx, [esp + 4]     ; address pointer

    and eax, 31            ; keep bit range 0-31

.try_lock:
    lock bts dword [edx], eax   ; set bit, CF = old value
    jc .spin                ; if already set, spin

    ret

.spin:
    pause

    bt dword [edx], eax    ; test bit
    jc .spin               ; still locked, continue spinning

    jmp .try_lock




; int try_acquire_lock(uint32_t *addr, uint32_t bit)
try_acquire_lock:
    mov eax, [esp + 8]     ; bit index
    mov edx, [esp + 4]     ; address pointer

    and eax, 31

    lock bts dword [edx], eax   ; set bit, CF = old value

    jc .fail                    ; already set → fail

    mov eax, 1                  ; success
    ret

.fail:
    xor eax, eax                ; return 0
    ret

; void release_lock(uint32_t *addr, uint32_t bit)
release_lock:
    mov eax, [esp + 8]     ; bit index
    mov edx, [esp + 4]     ; address pointer

    and eax, 31

    lock btr dword [edx], eax   ; clear bit
    ret