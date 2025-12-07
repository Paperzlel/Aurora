[bits 32]

global i386_gdt_load
i386_gdt_load:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]
    lgdt [eax]              ; Load GDT

    mov eax, [ebp + 12]
    push eax
    push .after
    retf                    ; Pushing the code segment and the next instruction to the stack allows us to far jump to the code segment and the rest of the code
                            ; retf pops IP then CS, and we need to reload the data segment so we do this as well
.after:

    mov ax, [ebp + 16]
    mov ds, ax              ; Change data segment and friends to use the new value
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, ebp
    pop ebp

    ret
