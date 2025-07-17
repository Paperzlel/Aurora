org 0x7c00
bits 16

%define ENDL 0x0d, 0x0a

start:
    jmp main

; 
; Prints a string to the screen
; Params:
; - ds:si points to the string
;
puts:
    ; Save registers we will modify, as they might contain data from other locations
    push si
    push ax

.loop:
    lodsb                   ; lodsb loads a byte from ds:si into al/ax/fax, then increments si
    or al, al               ; Any value NEQ to zero will return not zero here, so return if zero
    je .done

    mov ah, 0x0e            ; Ox0e prints the character in ah to the screen.
    mov bh, 0               ; Sets the page number (text mode) to 0
    int 0x10                ; Calls BIOS interrupt 0x10 which is a set of video output options

    jmp .loop

.done:
    pop ax
    pop si
    ret

main:

    ; Setup data segments
    mov ax, 0               ; Since ds/es can't be written to directly, we use an intermediate format.
    mov ds, ax
    mov es, ax              ; Both registers might have data in them so we clear first

    ; Setup stack
    mov ss, ax              ; Stack segment is 0x0000 (reading the offset, remember?)
    mov sp, 0x7c00          ; Stack pointer is at the start of the program
                            ; sp grows downwards (from 7c00 towards 0x0000) so we define it just before our application otherwise it overwrites
                            ; our program
    
    ; Move the message into the SI register
    mov si, msg_hello
    call puts               ; Call the function

    hlt

.halt:
    jmp .halt


msg_hello: db 'Hello, World!', ENDL, 0

times 510-($-$$) db 0
dw 0aa55h