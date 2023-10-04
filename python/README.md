# SimpleCPU Simulator Implemented in Python

## Install

To install and use the SimpleCPU simulator written in Python,
you'll need to clone the repo and use the [poetry][0] tool
to install it into a virtual environment.

```console
$ git clone http://github.com/JnyJny/SimpleCPU.git
$ cd SimpleCPU/python
$ poetry shell
<VE> $ poetry install
...
<VE> $ pasm ../docs/program_a.s
<VE> $ pdis program_a.o
00000000        get
00000001    copytox
00000002        get
00000003    copytoy
00000004        get
00000005       addx
00000006       addy
00000007        put 00000001
00000009        end
<VE> $ prun program_a.o
202
```

## `simplecpu` Subcommands

### simplecpu asm - Assembler

The `pasm` command will take SimpleCPU assembly (or text machine code representations)
and assemble them into a binary format. 

### simplecpu dis - Disassembler

The `pdis` command will print an textual assembly language representation of an
assembled file in binary format. 

### simplecpu run - Run a Program


[0]: https://python-poetry.org/docs/
