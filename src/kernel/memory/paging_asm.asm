[bits 32]

%define KERNEL_PAGE_TABLE_VIRTUAL_ADDRESS 0xc0100000

global __tlb_flush
__tlb_flush:
    mov eax, [esp + 4]
    invlpg [eax]
    ret
