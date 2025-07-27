# Information about the OS

## Main Addresses
- The bootloader loads the OS into memory at `0x2000:0000` which is `0x20000` in real terms. 
- The stack begins at position `0x07C0:0000` which is `0x07C00` in real terms.
- See [this link](https://wiki.osdev.org/Memory_Map_(x86)) for more information


## Stages
### Bootloader
This stage simply obtains some drive information and loads the rest of the kernel. Normally a bootloader would obtain more, but the majority of this
will occur during the very early stages of the kernel (prior to any `kmain()` function) when we are still not in protected mode.

### Early kernel
This stage has more room to play with and hence can load vastly more information about the kernel. However, it is still in real mode and can do a lot
of damage if not handled appropriately. Here, we want to do the following:
1. Print a help message
2. Enable A20 line
3. Setup GDTR
4. Move to 32-bit mode
5. Get the full memory map of the system
6. Setup COM connections for serial ports
7. Check CPU features
8. Enable graphics mode(s)

### Later kernel
This stage is currently under construction.


# Understanding Assembly

## Memory segmentation
Wacky old stuff means we use a system of `segment:offset`. Since this isn't actually the form we want, we need to understand some basics.
`real_address = segment * 16 + offset`, as the segment is stored as a hex value one row below the actual value.
For example, finding the main boot instruction (which is `7c00` on 16-bit mode) can be done both with `0x0000:0x7c00` or `0x7c0:0x0000`, for instance.

`ds` and `es` are data segments that can't be read and written to directly via machine constants.
`ss` is the stack segment and it contains the stack which we need to set up in most cases.

### Referencing a memory location

`segment:[base + index * scale + displacement]`
- Segment is any of the segment registers (`cs`, `ds`, `es`, `fs`, `gs` or `ss` but `ds` if unspecified)
- Base is `bp` or `bx` in 16-bit and any general-purpose register in 32/64-bit modes
- index is `si` or `di` in 16-bit but like base is any in 32/64-bit mode.
- scale is 32/64-bit mode only and is 1, 2, 4 or 8.
- displacement is a signed constant value like 12 or -3.

Example 1:
```asm
var: dw 100

    mov ax, var         ; Copies offset
    mov ax, [var]       ; Copies memory contents
```
Example 2:

```asm
array: dw 100, 200, 300     ; Variable name is the segment we are looking at FWIW

    mov bx, array           ; Copy the offset (first part of the array) into the register
    mov si, 2 * 2           ; Copy the offset within the array we want to get to by the size of the data (dw is 2 bytes)
                            ; This code reads array[2]
                            ; bx is the base and si is the index

    mov ax, [bx + si]       ; Copy memory contents into the ax register
```

## Interrupts

A signal to stop the processor from whatever it's doing and handle the signal.
Interrupts are caused by a) crappy code/exception, b) hardware (keyboard keys, timer tick, disk controller finishing) or c) via the `int` instruction in software.

There are a bunch of important BIOS-related interrupts, which I'll explain.
`int 10h` - Calls to video operations (printing to the screen)
`int 11h` - Equipment check (find what that means)
`int 12h` - Check the memory size
`int 13h` - Calls to a disk I/O operation. Used to load the rest of the kernel from the bootloader.
`int 14h` - Serial communications (find what this means)
`int 15h` - Cassette (find what this means)
`int 16h` - Keyboard I/O. For getting key inputs and so forth.
