# SimpleCPU Simulator Implemented in Python

## Install

To install and use the SimpleCPU simulator written in Python, you'll
need to clone the repo and use the [uv][0] tool to create a virtual
environment and install the project into it.

```console
$ git clone http://github.com/JnyJny/SimpleCPU.git
$ cd SimpleCPU/python
$ uv venv
$ uv sync
$ simplecpu asm ../docs/program_a.s
$ simplecpu dis program_a.o
00000000        get
00000001    copytox
00000002        get
00000003    copytoy
00000004        get
00000005       addx
00000006       addy
00000007        put 00000001
00000009        end
$ simplecpu run program_a.o
202
```

## `simplecpu` Subcommands

### simplecpu asm - Assembler

The `asm` subcommand will take SimpleCPU assembly (or text machine
code representations) and assemble them into a binary format.

### simplecpu dis - Disassembler

The `dis` subcommand will print an textual assembly language
representation of an assembled file in binary format.

### simplecpu run - Run a Program

TBD

[0]: https://github.com/astral-sh/uv
