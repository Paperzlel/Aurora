[bits 32]

; Macro definition for kernel load position
%define KERNEL_PHYSICAL_ADDRESS 0x100000
%define KERNEL_VIRTUAL_ADDRESS 0xc0100000
%define KERNEL_PAGE_TABLE_VIRTUAL_ADDRESS 0xc0000000
%define KERNEL_PAGE_TABLE_PHYSICAL_ADDRESS 0x00010000

section .kernel_lh.data align=4096

; Global page directory.
global page_directory
page_directory:
    times 1024 dq 0

; Page table for lower memory
lower_memory_table:
    times 1024 dq 0

; Page table for the kernel
kernel_table:
    times 1024 dq 0

section .kernel_lh.text

; Runs all the pre-C tasks before moving into the C kernel.
global start
start:
    ;; INITIALIZE PAGING
    mov esi, page_directory
    xor ecx, ecx

    ; Set all entries to be supervisor only. Userland directories should unset this flag.
_init_directory:
    or dword [esi], 0x00000002
    add esi, 4
    inc ecx
    cmp ecx, 1024
    jl _init_directory

    ; Set first entry (BIOS) to read from its page table
_post_dir_init:
    mov edx, lower_memory_table
    and edx, 0xfffff000
    or [page_directory], edx
    or dword [page_directory], 1        ; Enable page directory index 0

    xor ecx, ecx
    mov esi, lower_memory_table
    mov edi, ecx

    ; Identity map all pages from 0 --> 4MiB (this includes the loaded kernel)
_set_identity_pages:
    and edi, 0xfffff000
    or edi, 0x00000003
    mov [esi], edi
    add esi, 4
    add edi, 4096
    inc ecx
    cmp ecx, 1024
    jl _set_identity_pages

    ; Set EAX to find the index of the page directory to place the kernel
_post_identity_pages:
    mov eax, KERNEL_VIRTUAL_ADDRESS
    and eax, 0xffc00000
    shr eax, 22
    mov ecx, 4
    mul ecx

    ; Move to offset
    mov esi, page_directory
    add esi, eax

    mov edx, kernel_table
    and edx, 0xfffff000

    or [esi], edx
    or dword [esi], 1                   ; Enable page directory for the kernel

    xor ecx, ecx
    mov esi, kernel_table

    ; Calculate offset into the array to place our virtual data
    mov eax, KERNEL_VIRTUAL_ADDRESS
    and eax, 0x003ff000
    shr eax, 12
    mov ecx, eax
    mov edx, 4
    mul edx                             ; Find index into page table for this page

    add esi, eax
    mov edi, KERNEL_PHYSICAL_ADDRESS
    
    ; Set all addresses from 0x0010 0000 --> 0x003f ffff to 0xc010 0000 and upwards
_set_kernel_pages:
    and edi, 0xfffff000
    or edi, 0x00000003
    mov [esi], edi
    add esi, 4
    add edi, 4096
    inc ecx
    cmp ecx, 1024
    jl _set_kernel_pages

    ; Allocate memory for our dynamic page tables. They manually reside at 0x00010000, where our FAT filesystem was originally loaded.
    ; This setup gives us around 960 KiB of memory or just under 240 page tables (technically 239 page tables as the last byte would overlap
    ; with our page directory)
_setup_page_table_alloc:
    xor ecx, ecx
    mov esi, kernel_table
    mov edi, KERNEL_PAGE_TABLE_PHYSICAL_ADDRESS

    ; Set the tables
_set_allocated_page_tables:
    cmp edi, KERNEL_PHYSICAL_ADDRESS
    jge _cleanup_garbage_data                   ; Jump if the table already exists
    and edi, 0xfffff000
    or edi, 0x00000003
    mov [esi], edi
    add esi, 4
    add edi, 4096
    inc ecx
    cmp ecx, 1024
    jl _set_allocated_page_tables

    ; Set registers to clean up garbage data
_cleanup_garbage_data:
    mov edi, KERNEL_PAGE_TABLE_PHYSICAL_ADDRESS
    xor ecx, ecx

    ; Clean out all garbage data from memory
_clear_data:
    mov dword [edi], 0
    add edi, 4
    cmp edi, KERNEL_PHYSICAL_ADDRESS
    jl _clear_data

    ; Enable paging in CR3 and set flags in CR0, absolute jump to rest of kernel
_enable_paging:
    mov eax, page_directory
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax

    lea ebx, [_after_paging_jump]
    jmp ebx

section .text

; Symbol for C entrypoint
extern cstart

_after_paging_jump:
    ; Jump to C
    push dword [esp + 4]        ; Push boot info to the stack once again
    call cstart

    cli
    hlt
