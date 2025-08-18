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
    mov es, di                  ; 0x0000:0x0000 to fix an error on some BIOSes
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

    pop di
    pop esi
    pop bx
    pop es

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


global x86_Memory_GetMemoryRegion
x86_Memory_GetMemoryRegion:
    [bits 32]

    push ebp
    mov ebp, esp

    x86_prot_to_real
    
    [bits 16]

    push ebx
    push ecx
    push edx
    push esi
    push edi
    push es


    xor eax, eax
    mov eax, 0xe820
    mov ecx, 24
    mov edx, 0x534d4150

    mov bx, [bp + 8]

    linear_to_seg_ofs [bp + 16], es, edi, di

    int 0x15

    jnc .worked
    mov cl, 255             ; If carry flag is set, the function failed. Change the output to something silly to make it obvious it failed.

.worked:
    linear_to_seg_ofs [bp + 12], es, esi, si
    mov [es:si], bx
    mov al, cl
    
    pop es
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx

    push eax

    x86_real_to_prot

    [bits 32]

    pop eax

    mov esp, ebp
    pop ebp
    ret


global x86_VBE_GetVESAInfo
x86_VBE_GetVESAInfo:
    [bits 32]

    push ebp
    mov ebp, esp

    x86_prot_to_real

    [bits 16]

    push es
    push edi

    linear_to_seg_ofs [bp + 8], es, edi, di

    mov ax, 0x4f00
    int 0x10
    cmp al, 0x4f
    je .success
    
    xor eax, eax
    jmp .done

.success:
    mov ax, 1

.done:
    pop edi
    pop es

    push eax

    x86_real_to_prot

    [bits 32]

    pop eax

    mov esp, ebp
    pop ebp
    ret


global x86_VBE_GetVESAVideoModeInfo
x86_VBE_GetVESAVideoModeInfo:
    [bits 32]

    push ebp
    mov ebp, esp

    x86_prot_to_real

    [bits 16]

    push es
    push edi
    push ecx

    mov cx, [bp + 8]

    linear_to_seg_ofs [bp + 12], es, edi, di

    mov ax, 0x4f01
    int 0x10
    cmp al, 0x4f
    jne .failed

    and ah, 1
    jnz .failed
    
    mov eax, 1
    jmp .done

.failed:
    xor eax, eax

.done:
    pop ecx
    pop edi
    pop es

    push eax

    x86_real_to_prot

    [bits 32]

    pop eax

    mov esp, ebp
    pop ebp
    ret