global user_test_entry
global user_test_end

section .text

user_test_entry:
    mov ebx,0          ; counter

.loop:
    inc ebx

    mov eax, 1          
    int 0x1

    jmp .loop

user_test_end: