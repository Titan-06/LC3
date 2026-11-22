# Tests

Test programs for the LC-3 assembler and VM. Build both first (see the `Assembler/` and `VM/` folders), then run these from this folder.

## Test programs

| File | What it tests | Expected result |
|---|---|---|
| `hello.asm` | Basics: `LEA`, `PUTS`, `.STRINGZ`, `HALT` | Prints `Hello, World!`, then the VM prints `HALT` |
| `fibonacci.asm` | Loops, branches, `JSR`/`RET`, `LD`/`ST`, `OUT`, a decimal-printing subroutine | Prints `0 1 1 2 3 5 8 13 21 34 55 89`, then `HALT` |
| `2048.asm` | A large real program: stack-based subroutines, keyboard input via `KBSR`/`KBDR`, `.FILL` with labels, many `.STRINGZ` strings | A playable game of 2048 |
| `test.asm` | Assembler encoding only: every instruction, every directive, all number formats and all branch forms | Assembles to `test.obj`. **Do not run it**: it is not a real program |
| `error1.asm` | Offset out of range: `LEA` targets a label 301 words away (the 9-bit limit is +255) | `ERROR: Invalid Syntax at line 4`, no `.obj` created |
| `error2.asm` | Invalid register: `ADD R1, R8, #1` | `ERROR: Invalid Syntax at line 5`, no `.obj` created |