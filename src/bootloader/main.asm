org 0x7c00
bits 16

%define ENDL 0x0d, 0x0a

;
;   FAT12 header file
;
jmp short start                 ; First 3 bytes need to be EB 3C 90 (3c can be different) and this gets us to skip the
nop                             ;

bpb_oem: db 'MSWIN4.1'                  ; OEM identifier, 8 bytes 
bpb_bytes_per_sector: dw 512            ; No. of bytes per sector
bpb_sectors_per_cluster: db 1           ; No. of sectors per cluster
bpb_reserved_sectors: dw 1              ; No. of reserved sectors
bpb_fat_no: db 2                        ; No. of File Allocation Tables (FAT's) on the media.
bpb_dir_entry_count: dw 0e0h            ; No. of root directory entries
bpb_total_sectors: dw 2880              ; Total number of sectors (a 3.5" floppy has 1.44MB which is this in kB)
bpb_media_descriptor_type: db 0f0h      ; Media descriptor type (0x0f0 is 3.5" floppy disk)
bpb_sectors_per_fat: dw 9               ; No. of sectors per FAT disk (only FAT12, 9 sectors/fat on this disk model)
bpb_sectors_per_track: dw 18            ; Sectors per track (18 for a 1.44MB floppy)
bpb_heads: dw 2                         ; No. of heads per disk (2 for us as it's a double-sided floppy disk)
bpb_hidden_sectors: dd 0                ; No. of hidden sectors prior to this FAT volume. Leave zero for now
bpb_large_sector_count: dd 0            ; No. of logical sectors (leave blank for now)

; Extended boot record for BPB
bpb_drive_number: db 0                  ; Physical drive number, is zero as the drive is the first removable media.
bpb_reserved: db 0                      ; Reserved value in some instances
bpb_extended_boot_signature: db 29h     ; Boot signature, needs to be 0x29 for the next 3 entries to exist.
bpb_volume_id: db 12h, 24h, 36h, 48h    ; Volunme ID (serial number), can be anything we want :)
bpb_volume_label: db 'AURORA      '     ; Partition label, 11 bytes and needs to be padded for space
bpb_file_system_type: db 'FAT12   '     ; File system type, 8 bytes and padded with space


; Beginning of bootloader

start:

    ; Setup data segments
    mov ax, 0               ; Since ds/es can't be written to directly, we use an intermediate format.
    mov ds, ax
    mov es, ax              ; Both registers might have data in them so we clear first

    ; Setup stack
    cli                             ; Disable interrupts from bug on early 8088 CPUs
    mov ss, ax                      ; Stack segment is 0x0000 (reading the offset, remember?)
    mov sp, 0x7c00                  ; Stack pointer is at the start of the program
                                    ; sp grows downwards (from 7c00 towards 0x0000) so we define it just before our application otherwise it overwrites
                                    ; our program
    sti                             ; Re-enable interrupts

    
    mov [bpb_drive_number], dl      ; dl contains the current drive number, which should still be 0.

    mov ah, 0x02
    mov al, 1       ; sectors to read
    mov ch, 0       ; tracks to read
    mov cl, 2       ; sector no.
    mov dh, 0       ; head no.

    mov bx, 0x2000      ; Move to kernel space
    mov es, bx          ; es = 0x2000
    xor bx, bx          ; bx = 0 ==> ES:BX = 0x2000:0x0000

    stc                 ; Set carry flag (some BIOSes don't)
    int 13h             ; Call to read sector



    ; Move the message into the SI register
    mov si, msg_hello
    call puts               ; Call the function

    cli
    hlt

.halt:
    jmp .halt


;
;   Disk read errors
;
disk_read_error:
    mov si, msg_read_failed
    call puts
    jmp wait_and_reboot

wait_and_reboot:
    mov ah, 0
    int 16h
    jmp 0FFFFh:0            ; Jump to beginning of bios, which reboots.

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


msg_hello: db 'Hello, World!', ENDL, 0
msg_read_failed: db 'Reading from disk failed!', ENDL, 0

times 510-($-$$) db 0
dw 0aa55h