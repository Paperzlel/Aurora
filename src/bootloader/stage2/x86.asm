
section .text

%macro x86_prot_to_real 0 
    [bits 32]
    jmp word 0x18:.protected_mode16
    
.protected_mode16:
    [bits 16]

    mov eax, cr0
    and al, 0xfe
    mov cr0, eax

    jmp word 0x00:.real_mode

.real_mode:
    mov ax, 0
    mov ds, ax
    mov ss, ax

    sti

%endmacro

%macro x86_real_to_prot 0
    cli
    
    mov eax, cr0
    or al, 1
    mov cr0, eax

    jmp dword 0x08:.protected_mode

.protected_mode:
    [bits 32]

    mov ax, 0x10
    mov ds, ax
    mov ss, ax

%endmacro

;
; Converts a linear memory address to a segment:offset address. Useful when returning a pointer from an ASM function
; Parameters:
;   1 - Linear address (e.g. [bp + 8])
;   2 - Out target segment (e.g. es)
;   3 - Target 32-bit register to use (e.g. eax)
;   4 - Target lower 16-bit half of 3 (e.g. ax)
;

%macro linear_to_seg_ofs 4

    mov %3, %1
    shr %3, 4
    mov %2, %4
    mov %3, %1
    and %3, 0xf

%endmacro
