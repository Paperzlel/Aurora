# TODO.MD

## Bootloader

### Overall
- [ ] Add support for FAT16/FAT32 file types
- [ ] Add support for non-FAT file types
- [ ] Add video mode hint to VESA boot info

### Stage 1
- [x] Optimise file size for extra information

### Stage 2
- [x] Add support for ELF files for the kernel
- [ ] Add timestamp information
- [ ] Add serial port I/O configuration
- [ ] Implement detection of a keyboard for a boot requirement

## Kernel

### Architecture
- [x] Implement loading of the GDT again due to protected mode swapping
- [x] Create an IDT for error handling/IO messages
- [x] Add CPUID checks for different features
- [x] Add Virtual 8086 Mode option to prevent Real Mode/Protected mode switching
- [x] Abstract i686 instructions to generic hardware layer
- [x] Disable PIC/Check PIC drivers
- [x] Begin APIC drivers
- [x] Re-enable PIC for the time being
- [x] Modify CPU architecture so that different drivers are loaded at different times (i.e. GDT/IRQ/TSS should load instantly but not APIC)
- [ ] Move paging to use an architecture-specific implementation

### Core
- [ ] Reset stack after handing operation back to the kernel in C

### Debugging
- [ ] Implement a GDB stub for the OS to output info
- [ ] Add a `log_[error]()` macro that records the time (post-timer) and the error message to the screen

### Filesystem
- [ ] Implement a basic FAT12 filesystem driver
- [ ] Implement a VFS on top of the FAT12 driver
- [ ] Expose methods for `fopen` and `fread` in `stdio.h`

### HAL
- [ ] Add alternative timer settings that aren't PIT for specific systems
- [x] Mask out unused IRQ lines until needed
- [ ] Implement CMOS retrieval of the floppy disk controller

### Memory
- [x] Implement dynamic paging
- [x] Implement kernel-level malloc() and free()
- [x] Use MemoryRegions to find all regions where memory is used, and set blocks to invalid if so
- [x] Automatically identity page memory-mapped I/O data from MemoryRegions
- [ ] Align memory allocations to 16-byte boundaries
- [ ] Move pages to use a different memory address so we can re-use the FS (maybe the kernel load address?)

### Roadmap
- [x] Add paging
- [ ] Add a virtual filesystem
- [ ] Create module handling
- [ ] Create a file reader command (e.g. `rdfile`)

### Video
- [ ] Create a VBE graphics driver for hardware (current one is non-functional)
- [x] Move video drivers to use common resources like framebuffer info
- [x] Make stdio.c reroute to loaded graphics drivers
- [ ] Expose a video driver API for non-driver functions to use
- [ ] Load a charlist and draw chars to the screen

## Libc

- [ ] Implement current stubs as proper functions
- [ ] Compile libc as a hosted .so file
