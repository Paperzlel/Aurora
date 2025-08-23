[bits 32]

global __task_start
__task_start:
    mov al, 3
    mov bl, 2
    mul ax
    int 0xfe    ; instead of calling ret

global __task_end
__task_end: