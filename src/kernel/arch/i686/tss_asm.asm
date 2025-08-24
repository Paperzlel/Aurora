[bits 32]

global i686_tss_load
i686_tss_load:
    push ebp
    mov ebp, esp

    mov ax, [ebp + 8]
    ltr ax

    mov esp, ebp
    pop ebp
    ret