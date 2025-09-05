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

global i686_outb
i686_outb:
    mov al, [esp + 8]
    mov dx, [esp + 4]
    out dx, al
    ret
