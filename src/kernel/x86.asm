bits 16

section .text

;
; Call to BIOS to write a character
; args: character, page
;
global x86_Video_WriteCharTeletype
x86_Video_WriteCharTeletype:
    
    push bp
    mov bp, sp              ; Create new stack frame

    push bx                 ; Save bx register

    mov ah, 0x0e            ; Set to write a character to memory
    mov bh, [bp + 4]        ; [bp + 0] is the current call frame
    mov al, [bp + 6]        ; [bp + 2] is the return address (saved in words not bytes for 16-bit goodness)
                            ; [bp + 4] is the page number (since stack is pushed left-right)
                            ; [bp + 6] is the character to write.
                            ; This "shouldn't" work, as page and character are swapped for nanobyte, but this is actually intended behaviour - since
                            ; __cdecl works right-left, the second argument is added first, then the first.

    int 0x10                ; Write character to screen

    pop bx                  ; Restore bx

    mov sp, bp
    pop bp                  ; Restore previous stack frame
    ret
