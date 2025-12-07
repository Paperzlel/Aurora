bits 16

section .entry

extern __bss_start
extern __end

extern start                            ; C entry function using cdecl calling convention
global entry

entry:
    cli                                 ; Disable all hardware interrupts until the kernel is ready for them

    mov [a_boot_drive], dl              ; Save boot drive number for later

    mov ax, ds
    mov ss, ax
    mov sp, 0xfff0
    mov bp, sp

    ; Need to trigger/check A20 gate and enable GPT before moving to the rest of the bootloader

    push ax
    call check_a20 
    mov ah, 1
    cmp ah, al
    je .post_a20        ; If A20 is disabled, then we need to enable it. But if it's not, then there's no need to do so, and we can
                        ; move on with our lives.
    call enable_a20
.post_a20:
    call load_gdt       ; Load Global Descriptor Table, which will need to be re-configured and re-loaded when we reach the kernel 

    mov eax, cr0
    or eax, 1
    mov cr0, eax                            ; Set PE bit of CR0

    jmp dword 0x08:.protected_mode          ; Far jump into protected mode
                                            ; Code selector here is 0x08
.protected_mode:
    [bits 32]

    mov ax, 0x10                            ; Data selector is 0x10
    mov ds, ax
    mov ss, ax

    ; Clear all data in __bss so it is unintialized properly, instead of potential garbage values (this may not always happen)
    mov edi, __bss_start
    mov ecx, __end
    sub ecx, edi
    mov al, 0
    cld                         ; Clear direction flag (the registers we created in the GDT now point forwards)
    rep stosb                   ; Repeat the process until 0 has been reached (set from ecx - edi)

    xor edx, edx
    mov dl, [a_boot_drive]      ; Move boot
    push edx
    call start

    cli             ; Cancel hardware interrupts
    hlt             ; Pause forever

;
; Querys the keyboard in order to enable the A20 gate, so we can have the extended memory area for CPUs later than the 8086 (> 1 MiB).
; See also: https://wiki.osdev.org/I8042_PS/2_Controller 
; No parameters
; No returns
; No funerals
;
enable_a20:
    [bits 16]
    call a20_wait                       ; Wait for data to be sent
    mov al, 0xad                        ; Disable keyboard inputs
    out 0x64, al                        ; Send the command

    call a20_wait
    mov al, 0xd0
    out 0x64, al                        ; Read inputs from keyboard

    call a20_wait2                      ; See if output has been sent
    in al, 0x60                         ; Get data
    push ax

    call a20_wait
    mov al, 0xd1
    out 0x64, al                        ; Write to output

    call a20_wait
    pop ax
    or al, 2
    out 0x60, al                        ; Output to PS/2 controller the data read plus bit 1 which is the A20 gate (doing so enables the A20 gate)

    call a20_wait
    mov al, 0xae
    out 0x64, al                        ; Re-enable keyboard

    ret

a20_wait:
    [bits 16]
    in al, 0x64                         ; Bit 1 (the second bit) looks for input status. If busy, then we break from the function
    test al, 2
    jnz a20_wait
    ret

a20_wait2:
    [bits 16]
    in al, 0x64                         ; Bit 0 (the first bit) looks for if output is being sent. If set, then we can read data from port 0x60
    test al, 1
    jz a20_wait2
    ret

;
; Checks to see if the A20 gate is enabled or disabled, and if we need to enable it in the first place.
; No parameters
; Returns:
;   - 0 in al if A20 is disabled, and 1 in al if A20 is enabled
check_a20:
    [bits 16]       ; Because of protected mode we need to specify the size of the instructions here
    ; A20 is enabled if 0x0000:0x0500 and 0xffff:0x0500 are different when written to and disabled if the same
    push es
    push ds
    push si
    push di                 ; Push all used registers

    xor ax, ax
    mov ds, ax
    mov si, 0x0500          ; Effective address 0x0000:0x0500 = 0x00000500
    
    not ax
    mov es, ax
    mov di, 0x0510          ; Effective address 0xffff:0x0500 = 0x00100500

    mov al, [ds:si]
    mov byte [buffer_below_mb], al
    mov al, [es:di]
    mov byte [buffer_above_mb], al      ; Save values to memory

    mov ah, 1                           ; Predetermine outcome in ah
    mov byte [ds:si], 0
    mov byte [es:di], 1                 ; Write 0 and 1, if the memory wraps around then ds:si will also have 1 written to it
    mov al, [ds:si]
    cmp al, [es:di]                     ; If they're not equal then A20 is enabled, otherwise it is
    jne .exit
    dec ah

.exit:
    mov al, [buffer_below_mb]
    mov [ds:si], al
    mov al, [buffer_above_mb]           ; Restore values to previous value
    mov [es:di], al
    shr ax, 8                           ; Clear ax

    pop di
    pop si
    pop ds
    pop es
    ret

;
; Loads the GDT saved in memory to the CPU, to load our code and data segments properly
;
load_gdt:
    [bits 16]
    lgdt [gdt_desc]
    ret


a_boot_drive: db 0
buffer_below_mb: db 0
buffer_above_mb: db 0

; Global descriptor table, needed for protected mode.
; See http://www.osdever.net/tutorials/view/the-world-of-protected-mode for some insight on why each register is set as such
; Also see https://wiki.osdev.org/Global_Descriptor_Table for more info on how it's laid out
; Remember to read each doubleword right to left from the tables (little-endian)
a_gdt:
    dq 0                        ; NULL descriptor, 8 bytes

    ; 32-bit code segment
    dw 0x0FFFF                  ; Limit = 0xFFFFF, set rest of bytes in second doubleword
    dw 0

    db 0                        ; the quad is split into two doublewords. Base part A is bytes 0 - 15, part b 16 - 23.
    db 10011010b                ; Access byte (present, privilege level, type, executable, direction, read/write, access)
    db 11001111b                ; Flags (granularity (multiply limit by 4 kB for a 4GB limit), size (on for 32-bit), long mode (enabled, 64-bit))
                                ; plus end of limit (0x0F) for our full 32 bits
    db 0                        ; Last 8 bits of the base (32 bits total)

    ; 32-bit data segment
    dw 0x0FFFF                  ; Limit = 0xFFFFF, like above
    dw 0

    db 0                        ; Same as before
    db 10010010b                ; Similar privilege as above bytes except for bit 3 to make this a data segment
    db 11001111b                ; Same granularity and size as above
    db 0

    ; 16-bit code segment
    dw 0x0FFFF
    dw 0

    db 0
    db 10011010b                ; Same privilege
    db 00001111b                ; No granularity + 16-bit protected mode
    db 0

    ; 16-bit data segment
    dw 0x0FFFF
    dw 0

    db 0
    db 10010010b                ; Same privilege
    db 00001111b                ; No granularity + 16-bit protected mode
    db 0

gdt_desc:   dw gdt_desc - a_gdt - 1     ; Size of the GDT in bytes
            dd a_gdt                    ; Address of the GDT
