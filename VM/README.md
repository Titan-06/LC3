# LC-3 Virtual Machine

A virtual machine for the LC-3 (Little Computer 3) architecture, written in C in a single file, `lc3_vm.c`. It loads an assembled `.obj` program into memory and runs it.

## Build

Requires a POSIX system (Linux or macOS) and a C23-capable compiler.

```bash
gcc -Wall -o lc3_vm lc3_vm.c
```

## Usage

```bash
./lc3_vm path/to/program.obj
```

- Press `Ctrl+C` to quit. The terminal settings are restored automatically.
- The program must start at an address between `0x3000` and `0xFDFF`, otherwise the VM refuses to load it.

## Overview of `lc3_vm.c`

### Machine state
- `memory[MEMORY_MAX]` holds the 65,536 16-bit words of LC-3 memory.
- `registers[8]` holds the general-purpose registers `R0` to `R7`. The `GENERAL_REGISTERS` enum gives them names.
- `PC` is the program counter and `PSR` is the processor status register. Only its condition flags are used: `0b001` positive, `0b010` zero, `0b100` negative.
- The `OPCODES`, `TRAPS` and `SPECIAL_MEMORY_REGISTERS` enums name the opcodes, trap vectors and the keyboard registers (`KBSR`, `KBDR`).

### Loading a program
- `load_program` validates the arguments, opens the file, checks that it is not empty (using `get_file_size`), and reads the first word as the start address. It rejects start addresses outside the allowed range and programs too big to fit.
- LC-3 `.obj` files are big-endian, so every word is passed through `swap16` before being stored in `memory`.
- It returns the start address, or `0` on failure, which stops the VM.

### Main loop (`main`)
1. Install the `Ctrl+C` handler (`handle_termination`) and switch the terminal mode (`setup_noncanon_mode`).
2. Load the program, then set `PC` to its start address.
3. Repeat: fetch `memory[PC]`, increment `PC`, take the top 4 bits as the opcode, and call the matching `op_*` function in a `switch`.
4. Stop on `HALT`, `RTI` or an unknown opcode (which prints `BAD OPCODE`), then restore the terminal with `disable_noncanon_mode`.

### Helper functions
- `sext(num, sig_dig)` sign-extends a value with `sig_dig` significant bits to 16 bits. It is used for immediates and offsets.
- `setcc(number)` updates the condition flags in `PSR` after an instruction writes a register.
- `swap16(num)` swaps the two bytes of a word.

### Instructions
Each function decodes its own fields (registers, offsets, flags) from the 16-bit instruction using bit masks and shifts. Memory is accessed only through `mem_read` and `mem_write`.

### Memory-mapped keyboard
- `mem_read` handles `KBSR` (`0xFE00`) specially. It calls `check_key`, a non-blocking check that uses `select` to see whether input is waiting. If a key is waiting, it sets the ready bit in `KBSR` and stores the key in `KBDR` (`0xFE02`).
- `mem_write` ignores writes to `KBSR` and `KBDR`.

### Terminal handling
- `setup_noncanon_mode` turns off line buffering and echo with `termios`, so keys reach the program immediately. It saves the original settings first.
- `disable_noncanon_mode` restores them. It is called on normal exit and from `handle_termination` on `Ctrl+C`.

## Tests

Test programs for the VM are in the `Tests/` folder at the project root.