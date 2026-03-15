# SimpleCPU - Rust Implementation

A Rust implementation of the SimpleCPU simulator, following the Python
implementation (no IPC, single-process CPU + Memory).

## Build

```console
$ cargo build
```

## Usage

```console
$ cargo run -- run ../docs/program_b.s       # Run assembly source
$ cargo run -- run ../docs/program_b.s -t 50 # Custom timer interval
$ cargo run -- asm ../docs/sample1.s -o sample1.o  # Assemble to object file
$ cargo run -- run sample1.o                 # Run object file
```

## Architecture

Maps 1:1 to the Python implementation:

| Module       | Description                                    |
|-------------|------------------------------------------------|
| `main.rs`    | CLI (clap): run, asm subcommands              |
| `cpu.rs`     | CPU with registers, microcode, interrupts     |
| `memory.rs`  | 2000-word memory with bounds checking         |
| `opcode.rs`  | Instruction set definition (31 opcodes)       |
| `assembler.rs` | Source parser -> Memory                     |
| `constants.rs` | Mode, ProgramLoad, StackBase, MAGIC        |
| `error.rs`   | Error types via thiserror                     |

## TODO

- [ ] Disassembler subcommand
- [ ] Integration tests with sample programs
- [ ] Proper debug/trace output (equivalent to loguru levels in Python)
