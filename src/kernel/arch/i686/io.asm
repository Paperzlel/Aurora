[bits 32]

global i686_panic
i686_panic:
    cli
    hlt

global i686_inb
i686_inb:
    mov dx, [esp + 4]
    in al, dx
    ret

global i686_inw
i686_inw:
    mov dx, [esp + 4]
    in ax, dx
    ret

global i686_outb
i686_outb:
    mov al, [esp + 8]
    mov dx, [esp + 4]
    out dx, al
    ret

global i686_outw
i686_outw:
    mov ax, [esp + 8]
    mov dx, [esp + 4]
    out dx, ax
    ret

global i686_set_msr
i686_set_msr:
    mov ecx, [esp + 4]
    mov eax, [esp + 8]
    mov edx, [esp + 12]
    wrmsr
    ret

global i686_get_msr
i686_get_msr:
    mov ecx, [esp + 4]
    mov esi, [esp + 8]
    mov edi, [esp + 12]
    xor eax, eax
    xor edx, edx
    rdmsr
    mov [esi], eax
    mov [edi], edx
    ret