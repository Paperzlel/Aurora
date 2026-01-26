[bits 32]

global __tlb_flush
__tlb_flush:
    mov eax, [esp + 4]
    invlpg [eax]
    ret
