[bits 16]

global __vesa_start
__vesa_start:
    pop bx              ; Args are pushed onto the stack
    push cs
    pop ds

    mov ax, 0x4f02
    or bx, 0x4000       ; Assume linear framebuffer
    xor bx, 0x8000      ; Clear screen flag
    int 0x10

    cmp al, 0x4f
    mov al, 1
    je .end

    xor al, al
.end:
    
    push ax             ; Push return code
    int 0xfe

global __vesa_end
__vesa_end:
