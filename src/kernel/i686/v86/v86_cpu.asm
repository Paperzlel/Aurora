[bits 32]

global v86_enter_v86
v86_enter_v86:
    mov ebp, esp
    push 0
    push 0
    push 0
    push 0                      ; GS, FS, ES, DS

    push dword [ebp + 4]        ; SS
    push dword [ebp + 8]        ; ESP
    pushfd
    or dword [esp], (1 << 17)   ; Set VM flag
    push dword [ebp + 12]       ; CS
    push dword [ebp + 16]       ; EIP (now points to the new instruction set)
    iret

global v86_enter_v86_handler
v86_enter_v86_handler:
    push ebp
    mov ebp, esp

    push ebx                ; Save registers that may have values in them

    mov eax, [esp + 12]
    mov ebx, [esp + 16]
    mov ecx, [esp + 20]
    mov edx, [esp + 24]
    int 0xfd                ; Call interrupt for entry
    
    pop ebx                 ; Restore registers
    mov esp, ebp
    pop ebp
    ret

global v86_cpu_get_esp
v86_cpu_get_esp:
    mov eax, esp
    add eax, 4
    ret

global v86_cpu_get_eip
v86_cpu_get_eip:
    pop eax
    jmp eax