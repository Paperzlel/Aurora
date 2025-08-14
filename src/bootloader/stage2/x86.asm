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

global x86_Drive_GetDriveParameters
x86_Drive_GetDriveParameters:
    [bits 32]

    push ebp
    mov ebp, esp

    x86_prot_to_real
    [bits 16]

    push es
    push bx
    push esi
    push di

    mov dl, [bp + 8]            ; Get drive no.
    mov ah, 0x08
    mov di, 0
    mov es, di                  ; 0x0000:0x0000 to fix some bioses
    stc
    int 0x13

    mov eax, 1
    sbb eax, 0                  ; If CF is set, makes eax = 0

    linear_to_seg_ofs [bp + 12], es, esi, si
    mov [es:si], bl             ; Store drive type

    mov bh, cl
    mov bl, ch
    shr bh, 6
    inc bx

    linear_to_seg_ofs [bp + 16], es, esi, si
    mov [es:si], bx             ; Store cylinder count

    xor ch, ch
    and cl, 0x3f

    linear_to_seg_ofs [bp + 20], es, esi, si
    mov [es:si], cx             ; Store sector count

    mov cl, dh
    inc cx

    linear_to_seg_ofs [bp + 24], es, esi, si
    mov [es:si], cx             ; Store head count

    push di
    push esi
    push bx
    push es

    push eax

    x86_real_to_prot

    [bits 32]

    pop eax

    mov esp, ebp
    pop ebp
    ret

global x86_Drive_ReadDisk
x86_Drive_ReadDisk:
    [bits 32]

    push ebp
    mov ebp, esp

    x86_prot_to_real

    [bits 16]

    push es
    push ebx
    
    mov dl, [bp + 12]       ; Drive no.
    
    mov ch, [bp + 16]       ; 10 bits for cylinder, lower 8 here
    mov cl, [bp + 17]
    shl cl, 6               ; Get last 2 bits of cylinder no.

    mov al, [bp + 20]       ; Sector no.
    and al, 0x3f
    or cl, al               ; Add remaining cylinder bits

    mov al, [bp + 8]        ; Sector count

    mov dh, [bp + 24]       ; Head no.

    linear_to_seg_ofs [bp + 28], es, ebx, bx        ; Pointer to write to

    mov ah, 0x02
    stc
    int 0x13

    mov eax, 1
    sbb eax, 0

    pop ebx
    pop es

    push eax

    x86_real_to_prot

    [bits 32]

    pop eax

    mov esp, ebp
    pop ebp
    ret


global x86_Drive_ResetDisk
x86_Drive_ResetDisk:
    [bits 32]

    push ebp
    mov ebp, esp

    x86_prot_to_real

    [bits 16]

    mov dl, [bp + 8]
    mov ah, 0
    stc
    int 0x13

    mov eax, 1
    sbb eax, 0

    push eax

    x86_real_to_prot

    [bits 32]

    pop eax

    mov esp, ebp
    pop ebp
    ret