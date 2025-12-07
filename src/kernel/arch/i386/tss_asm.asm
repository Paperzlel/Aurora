[bits 32]

global i386_tss_load
i386_tss_load:
    push ebp
    mov ebp, esp

    mov ax, [ebp + 8]
    ltr ax

    mov esp, ebp
    pop ebp
    ret
