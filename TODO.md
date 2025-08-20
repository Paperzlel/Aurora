# TODO.MD

## Bootloader

### Overall
- [ ] Add support for FAT16/FAT32 file types
- [ ] Add support for non-FAT file types

### Stage 1
- [ ] Optimise file size for extra information

### Stage 2
- [x] Add support for ELF files for the kernel
- [ ] Add timestamp information
- [ ] Add serial port I/O configuration
- [ ] Implement detection of a keyboard for a boot requirement

## Kernel
- [ ] Implement loading of the GDT again due to protected mode swapping
- [ ] Create an IDT for error handling/IO messages
- [ ] Add CPUID checks for different features
- [ ] Add Virtual 8086 Mode option to prevent Real Mode/Protected mode switching
- [ ] Add paging
- [ ] Add a virtual filesystem
- [ ] Create module handling
- [ ] Create a file reader command (e.g. `rdfile`)
