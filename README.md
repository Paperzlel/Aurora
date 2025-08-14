# Project Aurora

My own OS, based on the work of nanobyte.

## Project Vision

We want to understand how an OS works, and this project is aimed at doing just that - to understand what exactly goes on when we load up our computers. Aurora is designed to be easy-to-read and easy-to-understand, at least with some amount of reading on the topic.

## To-do list

- [x] Create a basic printing function for Assembly without using kernel things
- [x] Create the bootloader that can load the OS from memory
- [x] Implement printf and other print family functions for our stage2 bootloader
- [x] Implement a C-based FAT driver to load the kernel
- [x] Load the kernel into memory
- [ ] Create basic serial port I/O
- [ ] Configure keyboard output from the user
- [ ] Read a file from a floppy disk and print its contents using a command (say `rdfile /test/test.txt`, for instance)


## Requirements
There are several different tools one needs. The command to install all of them is below:
```
$ sudo apt install build-essential make bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo nasm mtools qemu-system-x86 bochs-x bochsbios vgabios
```
Use other `apt` derivatives for other distros.

## Building

In order to make an operating system, one must first create the universe. This is not always enjoyable to follow as a process, so we've provided a `make` option to do that for you. Running `make toolchain` will install and build `binutils` and `gcc` from scratch, under a folder labelled `toolchain`. Ideally, this will compile custom versions of these tools to make OS development easier. If you experience errors during the process, go into your `Makefile` and change the version numbers for `binutils` and `gcc` until they work (downgrading may be better).

To build the OS itself, run `make` normally.

## Running with qemu 

Run the shell script `./run.sh` to enter qemu.

## Running with bochs

- Run `./debug.sh`
- navigate to View - Disassemble (or press Ctrl+D) and type `0x7C00` and press OK on both prompts
- double click on the first instruction to set a breakpoint. The line should turn red.
- press Continue
- if everything goes right, you will end up at address `0x7C00`. If the code looks different than boot.asm, run the Disassemble command again.

## Known errors
- Some parts of the code (e.g. the `org` instruction in the bootloader, call to `start`) will flag as an error when using NASM in VSCode. These are expected, as different binary formats have different errors and our bootloader is written as a flat binary whereas our kernel is written to `elf` standards. If they come up, don't panic! The compilers (should) know what they're doing and fix them for us.
- Currently, a hack to force code 