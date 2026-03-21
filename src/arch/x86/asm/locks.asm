extern acquireLock
extern releaseLock

;from osdev wiki
acquireLock:
    pop edx ; pops the dword addr
    pop eax ; pops the bit index
    and eax, 31

    ;ecx = (1<<eax)
    push 1
    mov cl, al
    shl dword [esp], cl
    pop ecx

    ;try
    lock bts [edx],eax
    jc .spin_wait
    ret

    
.spin_wait:

    ;check loop
    test dword [edx], ecx 
    pause
    jnz .spin_wait
    jmp acquireLock


releaseLock:
    pop edx ; pops the dword addr
    pop eax ; pops the bit index

    lock btr [edx],eax
    ret