org 0x7c00
bits 16

%define ENDL 0x0d, 0x0a

;
;   FAT12 header file
;
jmp short start                 ; First 3 bytes need to be EB 3C 90 (3c can be different) and this gets us to skip the
nop                             ; Bootloader Parameter Block (BPB) below.

bpb_oem: db 'MSWIN4.1'                  ; OEM identifier, 8 bytes 
bpb_bytes_per_sector: dw 512            ; No. of bytes per sector
bpb_sectors_per_cluster: db 1           ; No. of sectors per cluster
bpb_reserved_sectors: dw 1              ; No. of reserved sectors
bpb_fat_count: db 2                     ; No. of File Allocation Tables (FAT's) on the media.
bpb_root_entry_count: dw 0x0e0          ; No. of root directory entries
bpb_total_sectors: dw 2880              ; Total number of sectors (a 3.5" floppy has 1.44MB which is this in kB)
bpb_media_descriptor_type: db 0x0f0     ; Media descriptor type (0x0f0 is 3.5" floppy disk)
bpb_sectors_per_fat: dw 9               ; No. of sectors per FAT disk (only FAT12, 9 sectors/fat on this disk model)
bpb_sectors_per_track: dw 18            ; Sectors per track (18 for a 1.44MB floppy)
bpb_heads: dw 2                         ; No. of heads per disk (2 for us as it's a double-sided floppy disk)
bpb_hidden_sectors: dd 0                ; No. of hidden sectors prior to this FAT volume. Leave zero for now
bpb_large_sector_count: dd 0            ; No. of logical sectors (leave blank for now)

; Extended boot record for BPB
bpb_drive_number: db 0                      ; Physical drive number, is zero as the drive is the first removable media.
bpb_reserved: db 0                          ; Reserved value
bpb_extended_boot_signature: db 0x29        ; Boot signature, needs to be 0x29 for the next 3 entries to exist.
bpb_volume_id: db 0x12, 0x24, 0x36, 0x48    ; Volunme ID (serial number), can be anything we want :)
bpb_volume_label: db 'AURORA      '         ; Partition label, 11 bytes and needs to be padded for space
bpb_file_system_type: db 'FAT12   '         ; File system type, 8 bytes and padded with space


; BOOTLOADER

start:

    ; Setup data segments
    mov ax, 0               ; Since ds/es can't be written to directly, we use an intermediate format.
    mov ds, ax
    mov es, ax              ; Both registers might have data in them so we clear first

    ; Setup stack
    mov ss, ax                      ; Stack segment is 0x0000 (reading the offset, remember?)
    mov sp, 0x7c00                  ; Stack pointer is at the start of the program
                                    ; sp grows downwards (from 7c00 towards 0x0000) so we define it just before our application otherwise it overwrites
                                    ; our program

    mov [bpb_drive_number], dl      ; dl (drive number) set by BIOS

    push es
    mov ah, 0x08
    int 0x13                        ; Get drive information (the format may not be 100% correct)
    jc disk_read_error              ; cx now contains the cylinder and sector count
    pop es

    and cl, 0x3f                        ; Remove top 2 bits (we only want sector count)
    xor ch, ch
    mov [bpb_sectors_per_track], cx     ; Correct value

    inc dh                              ; last index of heads = head_count - 1, so increment
    mov [bpb_heads], dh
    xor dx, dx

    ; Calculate root directory sector count
    mov al, [bpb_root_entry_count]      ; Setup for multiplying root entry count
    shl ax, 5                           ; Faster to bit-shift than multiply

    mov bx, [bpb_bytes_per_sector]
    dec bx
    add ax, bx

    inc bx                              ; bx still has 511 as its value
    div bx                              ; ax now contains the needed value

    mov bx, ax                          ; Send value to bx as we need ax for the next step

    xor dx, dx                          ; Clear dx of quotient (it contains 511 after this operation and this messes up the CHS read later)


    ; Calculate the first data sector number (where non-root directories and files are stored)
    mov ah, [bpb_fat_count]
    mov al, [bpb_sectors_per_fat]
    mul ah

    add ax, [bpb_reserved_sectors]

    ; ax is the sector for the root directory, bx is the number of sectors in the root directory.
    add ax, bx
    mov [a_root_dir_sector_no], ax
    sub ax, bx                      ; first sector of cluster = (cluster - 2) * sectors_per_cluster + first_data_sector

    mov cl, 1
    mov bx, buffer                  ; Move the next place to read file data into (this is at 0x0000:0x7e00)
    mov dl, [bpb_drive_number]      ; Restore drive number if changed
    call read_disk

    ; Set string offset to start of the buffer
    xor bx, bx
    mov di, buffer

; Compare si and di and se if they are equal. When ch == cl, the two strings are equal and we have found stage 2.
.compare_value:
    mov si, stage2_name
    mov cx, 11                      ; Get length of the string
    push di
    repe cmpsb                      ; REPE = repeat while equal, CMPSB = compare matching bytes in es:di and ds:si. Increments SI and DI
    pop di
    je .found_stage2                ; Strings were equal, move to the next step

    ; String was not equal
    add di, 32
    inc bx
    cmp bx, [bpb_root_entry_count]
    jl .compare_value

    jmp stage2_not_found            ; If bx > root_entry_count, then we ran out of root directories. Move to error message

.found_stage2:

    ; Add 26 bytes to di and read the sector number into cx
    mov ax, [di + 26]
    mov [stage2_cluster], ax                ; Entry's first cluster number

    ; Now we have the actual sector data, we check the FAT to see how many clusters to load.

    mov ax, [bpb_reserved_sectors]
    mov bx, buffer
    mov cl, [bpb_sectors_per_fat]
    mov dl, [bpb_drive_number]
    call read_disk

    mov bx, STAGE2_LOAD_SEGMENT
    mov es, bx
    mov bx, STAGE2_LOAD_OFFSET          ; Set offset to 0x0000:0x0500 as the entrypoint for stage 2

.load_stage2_sectors:

    mov ax, [stage2_cluster]

    sub ax, 2                           ; first sector of cluster = (cluster - 2) * sectors_per_cluster + first_data_sector
    add ax, [a_root_dir_sector_no]      ; Re-obtain the first data sector; at some point we may need to mul with sectors_per_cluster

    mov cl, 1
    mov dl, [bpb_drive_number]      ; Re-conf
    call read_disk                  ; Read stage 2 into memory

    add bx, [bpb_bytes_per_sector]  ; Increment offset to read next sector in

    mov ax, [stage2_cluster]
    mov cx, 3
    mul cx
    mov cx, 2
    div cx                  ; Need active_cluster * 3/2
                            ; dx is now cluster no. mod 2

    mov si, buffer
    add si, ax
    mov ax, [ds:si]

    or dx, dx
    jz .even

.odd:                           ; Since dx mod 2 can be 1 or 0 it is hence even or odd and this changes how we deal with the next cluster
    shr ax, 4
    jmp .post_cluster_read

.even:
    and ax, 0x0fff

.post_cluster_read: 
    cmp ax, 0x0ff8                  ; End of cluster chain? If so, stop reading stage 2 into memory and move on
    jae .finish_read

    mov [stage2_cluster], ax        ; Set a new value for the cluster number
    jmp .load_stage2_sectors

; Found stage 2, now moving into it to boot properly
.finish_read:

    mov dl, [bpb_drive_number]
    mov ax, STAGE2_LOAD_SEGMENT                             ; Move data segment to the new location
    mov ds, ax
    mov es, ax                                              ; Set up registers to point to their new offset (FROM HERE ES AND DS SHOULD NOT CHANGE!!!!!)

    jmp STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET              ; Jump to stage 2

    jmp wait_and_reboot                                     ; Wait for any interrupts, should never happen as OS shutdown stays where it is

    cli
    hlt


;
;   File reading/writing
;

;
;   Converts Logic Block Addressing to Cylinder/Head/Sector addressing since floppy disks are numbered in LBA and CHS is for the interrupt.
;   Params:
;   - ax is the sector to read
;   - cx [bits 0-5]: sector no.
;   - cx [bits 6-15]: cylinder no.
;   - dh: head
;
lba_to_chs:

    push ax
    push dx

    xor dx, dx                              ; clear dx for operation
    div word [bpb_sectors_per_track]        ; ax = LBA / sectors_per_track (runs from [ax:dx] )
                                            ; dx = LBA % sectors_per_track
    
    inc dx                                  ; dx = (LBA % sectors_per_track) + 1 = sector
    mov cx, dx
    
    xor dx, dx
    div word [bpb_heads]                    ; ax = (LBA / sectors_per_track) / Heads = cylinder
                                            ; dx = (LBA / sectors_per_track) % Heads = head
    mov dh, dl                              ; dh = head
    mov ch, al                              ; ch = cylinder (lower 8 bits)
    shl ah, 6
    or cl, ah                               ; put upper 2 bits of cylinder in cl

    pop ax
    mov dl, al                              ; Restore dl
    pop ax
    ret

;
; Reads an LBA value from disk 
; Params:
; - ax contains the LBA value to use
; - cl contains the sector read count
; - dl contains the drive number
; - es:bx contains the address to store data at
read_disk:

    push ax
    push bx
    push cx
    push dx
    push di                     ; Push registers

    push cx                     ; Save cl
    call lba_to_chs             ; Compute CHS
    pop ax                      ; al = no. of sectors to read

    mov ah, 0x02                ; Save BIOS instruction
    mov di, 3                   ; Save retry count

.retry:
    pusha                       ; Panic-save all registers to stack
    stc                         ; Set carry flag
    int 0x13
    jnc .done                   ; Jump if no error

    popa
    call disk_reset

    dec di
    test di, di
    jnz .retry

.failed:
    jmp disk_read_error

.done:
    popa 
    pop di                      ; Pop manually saved registers
    pop dx
    pop cx
    pop bx
    pop ax
    ret


;
;   Error handling
;

; 
; Reboots if a disk reading error occurs
;
disk_read_error:
    mov si, msg_read_failed
    call puts
    jmp wait_and_reboot

;
; Resets disk controller
; Params:
; - dl contains drive number
;
disk_reset:
    pusha
    mov ah, 0
    stc
    int 0x13
    jc disk_read_error
    popa
    ret

;
; Force-jumps to the beginning of the BIOS and attempts to re-boot the PC
;
wait_and_reboot:
    mov ah, 0
    int 16h
    jmp 0x0FFFF:0            ; Jump to beginning of bios, which reboots.

.halt:
    cli
    hlt

;
; Prints an error message when stage 2 of the bootloader cannot be found at any point
;
stage2_not_found:
    mov si, msg_stage2_not_found
    call puts
    jmp wait_and_reboot


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


STAGE2_LOAD_SEGMENT     equ 0
STAGE2_LOAD_OFFSET      equ 0x0500      ; Load bootloader stage 2 to 0x0500 (lower in memory but usable)

msg_read_failed: db 'Read from disk failed!', ENDL, 0
msg_stage2_not_found: db 'Stage 2 not found!', ENDL, 0
stage2_name: db 'STAGE2  BIN'
stage2_cluster: dw 0
a_root_dir_sector_no: dw 0

times 510-($-$$) db 0
dw 0xaa55

buffer: