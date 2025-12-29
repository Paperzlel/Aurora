[bits 32]

global __vesa_start
__vesa_start:
    pop ebx
    push cs
    pop ds

    mov eax, 0x4f02
    or ebx, 0x4000      ; Assume linear framebuffer
    xor ebx, 0x8000     ; Clear screen flag
    int 0x10

    cmp al, 0x4f
    mov al, 1
    je .end

    xor al, al
.end:
    
    pop ebx
    push eax
    int 0xfe

global __vesa_end
__vesa_end:
