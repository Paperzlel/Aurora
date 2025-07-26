# Project Aurora

My own OS, based on the work of nanobyte.

## Project Vision

- [ ] Create a basic printing function for Assembly without using kernel things
- [x] Create the bootloader that can load the OS from memory
- [ ] Create a very basic I/O system


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