; global switch_proc_asm
; global load_proc_asm
; global setup_process_stack_asm


; extern switch_process

; section .data
;     proc_esp dd 0   

; section .text
; switch_proc_asm:
    
;     push dword [proc_esp]
;     sti
;     call switch_process      ; call function that rets next esp as eax and sets ebx as next ebp
;     cli
;     add esp, 4    ; clean up the argument off the stack in func

;     cmp eax,0

;     jne load_proc_asm 

;     popad                ; Restore registers
;     popfd                ; Restore flags
;     sti
;     iretd

; setup_process_stack_asm:
;     pop edx ;  esp
;     pop ebx ;  eip/code entry

;     mov ecx, esp
;     mov esp,edx

;     push dword 0x202   ;flags
;     push cs       ;cs needed but unused
;     push ebx      ;eip
;     pushad
;     pushfd

;     mov eax,esp ;return new process esp 
;     add eax,46 ;size of all pushed

;     mov esp, ecx 

;     ret 12
    

; load_proc_asm:
;     pop eax ;1st arg/esp
;     pop ebx ;2nd arg/ebp

;     mov ebp, ebx
;     mov esp, eax 

;     popad                ; Restore registers
;     popfd                ; Restore flags
;     iretd
    
