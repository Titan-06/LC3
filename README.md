# LC-3 Assembler and VM

An assembler and a virtual machine for the LC-3 (Little Computer 3), an educational 16-bit architecture. The assembler turns LC-3 assembly into machine code, and the VM runs that machine code.

## What's inside

### Assembler
- Reads LC-3 assembly and produces an object file (`.obj`) that the VM can load.
- Handles labels, directives and the standard LC-3 instruction set.

### Virtual Machine
- Emulates the LC-3: 16-bit words, 8 general-purpose registers, a program counter and condition flags.
- Implements the full LC-3 instruction set and the standard trap routines (keyboard and console I/O, halt).
- Trap routines are handled via built in C functions.

For build and usage instructions, look inside the `Assembler/` and `VM/` folders.

## Tested on

| | |
|---|---|
| OS | Fedora Linux 44 (x86_64) |
| Compiler | GCC 16.2.1 |
| Language standard | C23 |

I have only built and tested this on Linux. Other platforms or older compilers may need changes.

## Tests

I've also added tests for both the assembler and the VM, in the `Tests/` folder. They include some larger programs, such as 2048.

## References

- [Write your own Virtual Machine](https://www.jmeiners.com/lc3-vm/) (a reference whenever I got stuck)
- [Assembly lecture notes, CS 2461](https://cs2461-2020.github.io/lectures/assembly1.pdf) (the assembler reference)
- [LC-3 ISA specification](https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf) (the VM's instruction set reference)

## Credits

`2048.asm` in `Tests/` is taken from [rpendleton/lc3-2048](https://github.com/rpendleton/lc3-2048). All credit for it goes to the original author.