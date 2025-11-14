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
- [ ] Modify CPU architecture so that different drivers are loaded at different times (i.e. GDT/IRQ/TSS should load instantly but not APIC)

### Filesystem
- [ ] Implement a basic FAT12 filesystem driver
- [ ] Expose methods for `fopen` and `fread` in `stdio.h`

### Memory
- [x] Implement dynamic paging
- [ ] Implement kernel-level malloc() and free()
- [ ] Use MemoryRegions to find all regions where memory is used, and set blocks to invalid if so
- [ ] Automatically identity page memory-mapped I/O data from MemoryRegions
- [ ] Align memory allocations to 16-byte boundaries

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