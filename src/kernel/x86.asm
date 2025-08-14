section .text

global x86_outb
x86_outb:
    mov al, [esp + 8]
    mov dx, [esp + 4]
    out dx, al
    ret

global x86_inb
x86_inb:
    mov dx, [esp + 4]
    in al, dx
    ret
