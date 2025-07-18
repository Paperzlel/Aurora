org 0x0
bits 16

%define ENDL 0x0D, 0x0A

start:
    mov si, msg_hello
    call puts


.halt:
    cli             ; Cancel hardware interrupts
    hlt             ; Pause forever


;
; Prints a string to the screen
; Params:
; - ds:si points to the string
;
puts:
    ; Save registers we will modify, as they might contain data from other locations
    push si
    push ax
    push bx

.loop:
    lodsb                   ; lodsb loads a byte from ds:si into al/ax/fax, then increments si
    or al, al               ; Any value NEQ to zero will return not zero here, so return if zero
    je .done

    mov ah, 0x0e            ; Ox0e prints the character in ah to the screen.
    mov bh, 0               ; Sets the page number (text mode) to 0
    int 0x10                ; Calls BIOS interrupt 0x10 which is a set of video output options

    jmp .loop

.done:
    pop bx
    pop ax
    pop si
    ret

msg_hello: db 'Hello world from the kernel!', ENDL, 0