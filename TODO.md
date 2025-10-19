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
- [x] Implement loading of the GDT again due to protected mode swapping
- [x] Create an IDT for error handling/IO messages
- [x] Add CPUID checks for different features
- [x] Add Virtual 8086 Mode option to prevent Real Mode/Protected mode switching
- [ ] Create a VBE graphics driver
- [ ] Move video drivers to use common resources like framebuffer info
- [x] Abstract i686 instructions to generic hardware layer
- [ ] Abstract graphic drivers to use generic format
- [ ] Make stdio.c reroute to loaded graphics drivers
- [ ] Disable PIC/Check PIC drivers
- [ ] Enable/write APIC drivers
- [ ] Add paging
- [ ] Add a virtual filesystem
- [ ] Create module handling
- [ ] Create a file reader command (e.g. `rdfile`)
