[bits 32]

global i386_panic
i386_panic:
    cli
    hlt

global i386_inb
i386_inb:
    mov dx, [esp + 4]
    in al, dx
    ret

global i386_inw
i386_inw:
    mov dx, [esp + 4]
    in ax, dx
    ret

global i386_outb
i386_outb:
    mov al, [esp + 8]
    mov dx, [esp + 4]
    out dx, al
    ret

global i386_outw
i386_outw:
    mov ax, [esp + 8]
    mov dx, [esp + 4]
    out dx, ax
    ret

global i386_set_msr
i386_set_msr:
    mov ecx, [esp + 4]
    mov eax, [esp + 8]
    mov edx, [esp + 12]
    wrmsr
    ret

global i386_get_msr
i386_get_msr:
    mov ecx, [esp + 4]
    mov esi, [esp + 8]
    mov edi, [esp + 12]
    xor eax, eax
    xor edx, edx
    rdmsr
    mov [esi], eax
    mov [edi], edx
    ret