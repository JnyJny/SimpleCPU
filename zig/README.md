# SimpleCPU - Zig Implementation

A Zig implementation of the SimpleCPU simulator. The scaffold is complete
(types, build system, CLI, memory, assembler, interrupt handling) but most
opcode implementations in `src/cpu.zig` are left as `@panic("TODO")` stubs
for you to fill in.

## Build

```console
$ zig build
```

## Usage

```console
$ zig build run -- run ../docs/program_b.s       # Run assembly source
$ zig build run -- run ../docs/program_b.s -t 50 # Custom timer interval
$ zig build run -- asm ../docs/sample1.s -o sample1.o  # Assemble
```

## Architecture

Maps 1:1 to the Python and Rust implementations:

| Module          | Description                                    |
|----------------|------------------------------------------------|
| `main.zig`      | CLI: run, asm subcommands                     |
| `cpu.zig`       | CPU with registers, microcode, interrupts     |
| `memory.zig`    | 2000-word memory with bounds checking         |
| `opcode.zig`    | Instruction set enum (31 opcodes)             |
| `assembler.zig` | Source parser -> Memory                       |
| `constants.zig` | Mode, ProgramLoad, StackBase, MAGIC           |
| `error.zig`     | Error set                                     |

## What's Implemented

Already working (filled in as examples):
- `loadv`, `loada`, `loadi` - load instructions
- `put` - console output (port 1: integer, port 2: character)
- `jump` - unconditional jump
- `interrupt`, `ireturn` - system calls
- `end` - halt

Stubs for you to implement (marked with `@panic("TODO")`):
- Remaining loads: `loadx`, `loady`, `loadspx`
- Store: `store`
- I/O: `get` (random)
- Arithmetic: `addx`, `addy`, `subx`, `suby`
- Register copies: `copytox`, `copyfromx`, `copytoy`, `copyfromy`, `copytosp`, `copyfromsp`
- Control flow: `jumpeq`, `jumpne`, `call`, `ret`
- Stack: `incx`, `decx`, `push`, `pop`

Each TODO panic message describes what the opcode should do.
The Rust version in `../rust/src/cpu.rs` has the complete logic if you get stuck.

## Testing Your Implementation

```console
$ zig build run -- run test_minimal.s     # Should print: HI
$ zig build run -- run ../docs/sample1.s  # Should print: ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678910
$ zig build run -- run ../docs/program_b.s # Full test (needs call/ret)
```

## Notes on Zig 0.15

This targets Zig 0.15.2. Some things that differ from older tutorials:
- `std.io.getStdOut()` is gone; use `std.fs.File{ .handle = std.posix.STDOUT_FILENO }`
- `File.writer()` now requires a buffer argument; for simple output, use `File.writeAll()`
- `std.debug.print()` is the easy path for stderr output
- `std.fmt.bufPrint()` for formatted integer-to-string conversion
- Build files use `b.createModule()` + `.root_module` instead of `.root_source_file`
