# Simple CPU Simulator Implemented in C

## Building

The Makefile will build all the components of the CPU simulator as
laid out in the problem specification (../docs/project_1.docx).

The components are:
- memory: a process which mediates access to simulated memory
- cpu: a process which executes programs stored in memory
- loader: stores programs into memory and starts the CPU executing

```console
$ make 
cc -c io.c
cc -c instructions.c
cc -c cpu.c
cc -o cpu io.o instructions.o cpu.o
cc -c memory.c
cc -o memory memory.o 
cc -c loader.c
cc -o loader loader.o io.o
$ 
```

## Running

The `loader` is the front end to the CPU and memory. 

```console
$ ./loader -h
usage: loader -f program_file [-d]
```

The program file argument is the path to a file containing a text
representation of the CPU's assembly language. The `-d` switch enables
debug mode on the CPU and will dump machine state at after the instruction
fetch and before the instruction execution.

```console
$ ./loader -f sample1.s -d
[LOAD] program1.s
[ MEM] Starting.
[0000] 0001 0072 0009 0002 0001 0073 0009 0002 0023 0011 
[0010] 0050 0001 0010 0009 0002 0024 0000 0000 0000 0000 
[ CPU] Start. Timer 100 Debug 1
--------------------------------------------------
[ir] 00000001 [pc] 00000000 [sp] 00000999 [mode] USER
[ac] 00000000 [ x] 00000000 [ y] 00000000 [DEBG] ON
[ti] 00000100 [cy] 00000000 [t?]        1 [INTR] ON
--------------------------------------------------
[ir] 00000009 [pc] 00000002 [sp] 00000999 [mode] USER
[ac] 00000072 [ x] 00000000 [ y] 00000000 [DEBG] ON
[ti] 00000100 [cy] 00000001 [t?]        0 [INTR] ON
[COUT] port=2 AC[72]=H
--------------------------------------------------
[ir] 00000001 [pc] 00000004 [sp] 00000999 [mode] USER
[ac] 00000072 [ x] 00000000 [ y] 00000000 [DEBG] ON
[ti] 00000100 [cy] 00000002 [t?]        0 [INTR] ON
--------------------------------------------------
[ir] 00000009 [pc] 00000006 [sp] 00000999 [mode] USER
[ac] 00000073 [ x] 00000000 [ y] 00000000 [DEBG] ON
[ti] 00000100 [cy] 00000003 [t?]        0 [INTR] ON
[COUT] port=2 AC[73]=I
--------------------------------------------------
[ir] 00000023 [pc] 00000008 [sp] 00000999 [mode] USER
[ac] 00000073 [ x] 00000000 [ y] 00000000 [DEBG] ON
[ti] 00000100 [cy] 00000004 [t?]        0 [INTR] ON
--------------------------------------------------
[ir] 00000001 [pc] 00000011 [sp] 00000998 [mode] USER
[ac] 00000073 [ x] 00000000 [ y] 00000000 [DEBG] ON
[ti] 00000100 [cy] 00000005 [t?]        0 [INTR] ON
[USTK] 0999: 0010
--------------------------------------------------
[ir] 00000009 [pc] 00000013 [sp] 00000998 [mode] USER
[ac] 00000010 [ x] 00000000 [ y] 00000000 [DEBG] ON
[ti] 00000100 [cy] 00000006 [t?]        0 [INTR] ON
[USTK] 0999: 0010
[COUT] port=2 AC[10]=

--------------------------------------------------
[ir] 00000024 [pc] 00000015 [sp] 00000998 [mode] USER
[ac] 00000010 [ x] 00000000 [ y] 00000000 [DEBG] ON
[ti] 00000100 [cy] 00000007 [t?]        0 [INTR] ON
[USTK] 0999: 0010
--------------------------------------------------
[ir] 00000050 [pc] 00000010 [sp] 00000999 [mode] USER
[ac] 00000010 [ x] 00000000 [ y] 00000000 [DEBG] ON
[ti] 00000100 [cy] 00000008 [t?]        0 [INTR] ON
[ CPU] End. Executed 9 instructions.
```
