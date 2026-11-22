# LC-3 Assembler

A two-pass assembler written in C. It converts LC-3 assembly (`.asm`) into an object file (`.obj`) that can be run on the LC-3 VM in this repository.

## Build

Needs GCC and `make` (developed and tested on Linux).

```bash
make          # builds ./assemble
make clean    # removes ./assemble
```

## Usage

```bash
./assemble path/to/program.asm
```

This writes `program.obj` next to the source file and prints `Obj File Created: ...`. Errors are printed with the line number. You can then run the resulting `.obj` file with the VM, but make sure to go through the limitations :3

## Supported syntax

| | |
|---|---|
| Instructions | `ADD AND NOT BR JMP RET JSR JSRR LD LDI LDR LEA ST STI STR TRAP RTI` |
| Branch forms | `BR BRn BRz BRp BRnz BRnp BRzp BRnzp` |
| Trap aliases | `GETC OUT PUTS IN PUTSP HALT` |
| Directives | `.ORIG .END .FILL .BLKW .STRINGZ` |
| Registers | `R0` to `R7` |
| Numbers | `#10` or `10` (decimal), `x3000` (hex), `b1010` (binary) |
| Comments | `;` to the end of the line |
| Whitespace | spaces or tabs between tokens, commas between operands, Unix or Windows line endings |

## How the code is organised

### `main.c`: driver, symbol table and the two passes
- **Symbol table:** `add_symbol_entry`, `is_symbol` and `free_symbol_table` store each label with its address.
- **`strip_comment`:** cuts a line at `;`, unless the `;` is inside a string.
- **`first_pass`:** reads the file once and tracks the address counter (set by `.ORIG`). Labels are held until the next instruction or directive appears, then saved with that address. The counter moves by 1 per instruction, or by the size of a directive (`is_directive`). It reports a bad `.ORIG`, bad directive parameters and duplicate labels.
- **`second_pass`:** reads the file again. It writes the origin as the first word, then for each line it collects the tokens and replaces labels with PC-relative offsets. For `.FILL` the label's absolute address is used instead. It then calls `handle_conversion` for instructions or `handle_directive` for directives, and stops at `.END`.
- **`add_instr_buffer` and `write_binary`:** each word is byte-swapped to big-endian and appended to a buffer. The buffer is written to `<name>.obj`.
- **`main`:** runs the first pass, then the second.

### `helpers.h` / `helpers.c`: shared types and utilities
- `helpers.h` defines the opcode, trap and directive enums, the `DELIMS` / `OPERAND_DELIMS` token separators, and two dynamic arrays: `list` (strings) and `short_int_list` (16-bit words).
- `helpers.c` holds the `valid_tokens` and `token_code` tables that map each of the 33 mnemonics to its code, and the list functions (`add_list_element`, `clear_list`, `add_short_int_list`, `clear_short_int_list`). It also has:
  - `valid_token`: looks up a mnemonic.
  - `is_directive`: returns how many words a directive occupies.
  - `trim_spaces`: strips surrounding spaces, tabs and carriage returns.
  - `decode_string`: handles escape sequences.
  - `is_in_range`: checks that a signed value fits in a given number of bits.

### `conversion.h` / `conversion.c`: turning tokens into machine words
- `is_num` parses numbers, a register parser handles `R0` to `R7`, and `trim_bits` masks a value to a field width.
- One encoder per instruction format:
  - `add_and`
  - `br`
  - `indirect_traps` (`TRAP x25`)
  - `direct_traps` (`HALT`, `PUTS`, etc.)
  - `jmp_jsrr`
  - `jsr_ret`
  - `pc_offset9` (`LD`, `LDI`, `LEA`, `ST`, `STI`)
  - `ldr_str_not`
  - `rti`
- `handle_conversion` picks the right encoder from the token code and returns `0` on a syntax error.
- `handle_directive` expands `.STRINGZ`, `.BLKW` and `.FILL` into words.

### `Makefile`
Compiles the three `.c` files with `-Wall` into `assemble`. It links the math library (`-lm`) because `is_in_range` uses `pow`.

## Limitations

- **Case matters.** Mnemonics, directives and registers must be uppercase. Branch flags must be lowercase and in `n`, `z`, `p` order (`BRnz`, not `BRzn`). Lines using lowercase mnemonics are not recognised as instructions.
- **Labels:** no colon after the name (`LOOP`, not `LOOP:`), case-sensitive, and they can't share a name with a mnemonic. A label can sit on its own line or before an instruction. Only one instruction or directive per line, and lines must be under 255 characters.
- **No `0x` prefix.** Write hex as `x3000`.
- **Plain `BR` has no condition bits.** The VM here treats that as an unconditional branch, but other LC-3 simulators treat it as never taken. Use `BRnzp` if the `.obj` may run elsewhere.
- **Value ranges:** `ADD`/`AND` immediates are -16 to 15, branch and `LD`/`LDI`/`LEA`/`ST`/`STI` offsets are -256 to 255, and `JSR` offsets are -1024 to 1023.
- **One program per file:** a single `.ORIG` before anything else, and a closing `.END` is required. Anything after `.END` is ignored.
- **Strings:** `.STRINGZ` stores one character per word(One 16 bit memory addresss) (use `PUTS`, not `PUTSP`) and supports `\n \t \r \e \0 \\ \"`.
- **File names:** the source needs an extension (e.g. `.asm`), and its path must be under 60 characters.
- **VM compatibility:** the assembler accepts any origin, but the VM only loads programs starting between `x3000` and `xFDFF`. `RTI` assembles but is not executed by the VM.
- **Byte order:** output is the standard big-endian `.obj` format, and the writer assumes a little-endian host (such as x86).