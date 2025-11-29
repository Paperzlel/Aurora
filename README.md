# Aurora
A custom operating system designed for i386 architectures and based on the work of nanobyte.

## Project Vision

### Overview
As a naturally curious person, I wanted to find out how operating systems worked. Rather than comb my way through the Linux kernel (which whilst a good source, is not for the faint of heart), I wanted to learn the hard way - by creating something of my own. 

Because of this aim, Aurora is designed to be clear in what it's doing - at least to the extent of understanding what the code does (the why part would make the majority of the repository comments, and that is more the task of a YouTube series - see nanobyte's for an idea of what I'm talking about). The code may not be fast, but what is sacrifices in that is readability and digestion.

### Short-term goals
As of writing, the goal for the project is fairly simple: create a basic shell window, and create a command to display the contents of a text file into the shell window. 

Whilst on paper this seems fairly simple, in practice there is plenty more to do than just this - at time of writing, the bootloader has only just been finished and the kernel is little more than a *"Hello, World!"* in the terminal. Many other items, displayed out in the sections below, need to be accomplished before the majority of the aforementioned code can be written.

### Long-term goals
I would lie if I said that there was no greater goal than the short-term ones, however many of these are simply aspirational and unlikely to be accomplished in a human lifetime. This does not mean they will never happen, just that they are unlikely to be done any time soon.

- Create a multi-drive bootloader (i.e. booting from disk image/hard drive)
- Make the project bootable on real-world media
- Create an Intel/AMD GPU driver
- Create an NVidia driver (less likely than above, lack of documentation)
- Create a compiler/C extension for GCC to compile on-disk

## To-do list
See [this list](https://www.github.com/Paperzlel/Aurora/blob/main/TODO.md) for a full rundown of what has been done and still needs to be done. This is not an exclusive list and may or may not be updated as the project goes on.

## Requirements
The command(s) below will install the majority of the required packages for building.

**Debian-based:**
```
$ sudo apt install build-essential make bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo nasm mtools qemu-system-x86 alien autoconf2.69
```
(TODO: Update this package list for non-Debian distros; help is appreciated with updating this package list)

### Installing bochs
To install `bochs`, go to [here](https://sourceforge.net/projects/bochs/) and grab the latest RPM package (I personally recommend 3.0). Then once it's installed, go to its downloaded folder and type the following commands:
```
sudo alien bochs-3.0-1.x86_64.rpm
sudo dpkg -i [new_package_name].deb
```
This should install the latest version of `bochs` for you properly. We do this as the current `bochs` package on `apt` is version 2.7, which has known issues with its VESA VBE BIOS, and since we use those instructions in the bootloader we require this version of `bochs`. Sorry.

*Please note: This **may** require you to install a newer version of your distro. Please do so if needed.*

## Building

### Toolchain (REQUIRED)
In order to make an operating system, one must first create the universe. This is not always enjoyable to follow as a process, so we've provided a `make` option to create the universe for you. Running `make toolchain` will install and build `binutils` and `gcc` from scratch, under a folder labelled `toolchain`. These are custom versions specific to your PC, which are compiled in such a manner so that they contain no default references or depedencies and only use the code that we have strictly written. Patches for both `binutils` and `gcc` are added which prepare the tools for use with Aurora specifically.

If you experience errors during the process, go into your `Makefile` and change the version numbers for `binutils` and `gcc` until they work (downgrading may be better for some people, however this may bring up issues down the line). There may also be an issue with custom git patches not being applied, in which case I will attempt to fix those as soon as possible

### Operating System
To build the OS itself, run `make` normally. This will create 3 binary files: `stage1.bin`, `stage2.bin` and `kernel.bin`, as well as `.map` files for the kernel and stage 2. The entire OS itself is stored inside `main_floppy.img`.

## Running with qemu + GDB

QEMU and GDB have been set up to work together within Visual Studio Code using their debugger. Adding breakpoints within C code and pressing F5 to run will allow the project to be debugged as normal.

For any extra references, see [this link](https://stackoverflow.com/questions/1471226/most-tricky-useful-commands-for-gdb-debugger).

## Running with bochs

1. Run `./debug.sh`
2. Navigate to View - Disassemble (or press Ctrl+D) and type `0x7C00` and press OK on both prompts
3. Double-click on the first instruction to set a breakpoint. The line should turn red.
4. Press Continue
5. If everything goes right, you will end up at address `0x7C00`. If the code looks different than boot.asm, run the Disassemble command again.
6. Repeat this for any other ASM code you may wish to debug, using the generated `.map` files for reference. 

To view memory at any given time, press F7 and type in the address you wish to observe. From there, you can see all of the memory from that given address and its respective values

## Contributing
Anyone wishing to contribute code is welcome to do so, however do bear in mind that the project in its current form is still missing a vast majority of features and each feature will need to be considered heavily before being implemented. Contributing guidelines, pull reqest formatting, proposals and so forth will be formalized when there is need for such a process.