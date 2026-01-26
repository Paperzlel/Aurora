# TODO.MD

## Bootloader

### Overall
- [ ] Add support for FAT16/FAT32 file types
- [ ] Add support for non-FAT file types
- [x] Add video mode hint to VESA boot info

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
- [ ] Improve terminal handling by storing messages in a buffer that is flushed to disk every N bytes (writing to a file)

### Debugging
- [ ] Implement a GDB stub for the OS to output info
- [x] Add a `log_[error]()` macro that records the time (post-timer) and the error message to the screen

### Filesystem
- [ ] Abstract reading from disk and the filesystem
- [ ] Add "mountpoints" where the non-boot drives host their files
- [x] Implement a basic FAT12 filesystem driver
- [x] Implement a basic VFS on top of the FAT12 driver

### HAL
- [ ] Add alternative timer settings that aren't PIT for specific systems
- [x] Mask out unused IRQ lines until needed
- [x] Implement CMOS retrieval of the floppy disk controller
- [x] Add a timer sleep function that waits for a given period of time
- [ ] Handle invalid CHS/LBA values in the floppy driver

### Memory
- [x] Implement dynamic paging
- [x] Implement kernel-level malloc() and free()
- [x] Use MemoryRegions to find all regions where memory is used, and set blocks to invalid if so
- [x] Automatically identity page memory-mapped I/O data from MemoryRegions
- [x] Align memory allocations to 16-byte boundaries
- [ ] Move pages to use a different memory address so we can re-use the FS (maybe the kernel load address?)
- [x] Separate kernel memory data and kernel libraries from user code (re-map kernel to higher half?)
- [x] Properly manage physical memory (no current use of the memory map)
- [x] Implement `realloc()` for the kernel (basically needed from here)
- [ ] Decide on either removing unused tables (faster; more memory usability) or using a bitmask
- [x] Ensure that heap headers are aligned to the start of a page (decided to not do, heaps are handled differently now)
- [ ] Add userspace allocations (uses different memory beginnings and permissions)
- [ ] Allocate the other 768 missing page tables (totals to 3MiB, have 1.8MiB for FB and 127MiB elsewhere)
- [ ] Set proper R/W bits on pages that can and can't be written to

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
- [ ] Expose methods for `fopen` and `fread` in `stdio.h`
- [ ] Compile libc as a hosted .so file