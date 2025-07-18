# Project Aurora

My own OS, based on the work of nanobyte.

## Project Vision

- [ ] Create a basic printing function for Assembly without using kernel things
- [ ] Create the bootloader that can load the OS from memory
- [ ] Create a very basic I/O system


## Building/Running

Use `make` to build.

For running the application, call `./run.sh`. For debugging, call `./debug.sh`.

## Running with bochs

- run `./debug.sh`
- navigate to View - Disassemble (or press Ctrl+D) and type `0x7C00` and press OK on both prompts
- double click on the first instruction to set a breakpoint. The line should turn red.
- press Continue
- if everything goes right, you will end up at address `0x7C00`. If the code looks different than boot.asm, run the Disassemble command again.