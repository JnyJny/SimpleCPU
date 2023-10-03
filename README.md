# SimpleCPU Simulator

This repo houses two separate implementations of a simple CPU
architecture simulator (SimpleCPU) that is described in moderate
detail in the the [docs][0] folder. The first implementation of
SimpleCPU is in C and follows the problem description relatively
closely; the CPU and memory modules communicate via pipes as
specified. The Python implementation of SimpleCPU does not use pipes
because the interprocess communication (IPC) aspect was not very
interesting to me so I skipped it on the second go around.

## CPU Simulators

CPU simulators are fun to write and give you a better idea of how
simple a computer can be and still be useful. Our simple computer can
be thought of as a set of scratch memory locations, called registers
and the logic that moves values from memory into the registers,
performs operations on the data in the registers and move data out of
the registers back to memory. Memory in this implementation is an
array of signed integer values that exist in a small linear address
space of 2000 words.

### Registers

Most computer architectures have a few registers in common. They may
be named differently but they perform that same function. These
registers are the Program Counter (PC), the Instruction Register (IR),
and the Stack Pointer (SP). These three register tell the computer
where it is at in the program, PC, what instruction is being executed (IR)
and where the top of the program stack is (SP).


### Backtrack: What's a Program?

Programs are a list of instructions that tell the computer what
operations to "execute" and the order to perform them in. The
instructions in memory are simplified version of the source code,
often called operation codes or opcodes. Programs that people write
are transformed with programs called assemblers, compilers or
interpreters into opcodes that computers can decode and execute.

The programs that this simulated CPU and memory are expected to
execute are found in the docs folder and also in the c and python
folders. Our programs are written in assembly syntax, which is
specific to this CPU implementation. Higher level languages, like C,
use compilers to turn C source code into an intermediate language
which is transformed again into machine code that the target CPU can
execute. For our purposes, we will use tools called `asm` or `pasm` to
assemble source code (files with a `.s` suffix are traditionally
denote assembly code files) into object code files whose suffixes will
be `.o`. The reverse operation is often called disassembly and there
are two tools `dis` and `pdis` that will provide a human view of
the code in an object file. 

### More Registers

Our simulated CPU has a couple of more registers we need to talk
about.  First is the Accumulator (AC) register, which is a scratch
register where the results of many operations are stored. There are
two other multi-purpose registers, X and Y which are used by various
instructions. All of these registers are readable and writable,
subject to the rules of the instructions that use the registers.

There are several registers that are read-only (either enforced or by
custom). The `cycles` register in my implementation is a free running
counter that keeps track of the number of instrutions executed. The
`mode` register determines what capabilities are available to the
instructions currently executing (whether they are in User mode or
System mode). There is a register `interrupts_enabled` that determines
whether interrupts are enabled or disabled. Interrupts are a switch
from User mode to System mode, sometimes triggered asynchronously by
external events (like the timer interrupt) or synchronously by
executing the interrupt instruction in a program. 


### Instructions

The SimpleCPU implements 31 instructions, some of which are followed
by an operand in memory. Some instructions perform read and write
operations to memory. There are instructions for pushing and popping
values from the stack. There are console IO instrutions, some simple math
instructions, program control flow instructions and an end instruction which
causes the CPU simulator to exit. All instructions have some sort of effect
on the state of the CPU, described by its set of registers. 

### The Fetch, Decode, Execute Loop

Computers today are pretty complex machines, however all computers embody
a loop that does the following things:

1. Fetch the current instruction at PC into the IR register
2. Decode the instruction
3. Execute the instruction
4. Go back to 1.

Typically, each of these steps would take place on the ticking of the
CPU clock which is an electrical signal distributed throughout the
CPU. The clock signal is the heartbeat of the machine and causes
operations to occur at defined intervals. This helps avoid conflicting
operations that could clobber the state of the CPU and crash the
running program.  The closest my simulator comes to a clock is the
`cycles` register that is incremented for each instruction executed,
however it doesn't regulate any other operations in the CPU (ok maybe
the timer interrupt).

Modern computers typically have several fetch-decode-execute loops
running simultaneously and out of phase with the goal of executing an
instruction every clock cycle. These types of instruction execution
architectures are very complex and can result in some very weird
behaviors depending on the stream of instructions being executed.

#### Fetch

Fetching the instruction is a read operation on memory, using the
address stored in the PC register to determine where the read happens.
The results of the read are stored into the IR register, which is 
not visible to any of the defined instructions.

#### Decode 

Once the instruction is available in the IR, the computer then decodes
the opcode to determine which instruction it is and if it requires any
other data to execute (loading an operand, switching to SYSTEM mode,
etc).

#### Execute

Executing an instruction results in changes to the registers (the
state of the CPU) which becomes the state of the machine for the
execution of the next instruction.

The PC register will change, depending on the type of instruction that
was executed.  One class of instructions is known collectively as
Control Transfer Instructions or CTIs. The CTIs in our simple
architecture are; 'jump', 'jumpeq', 'jumpne', 'call', 'interrupt',
'ret' and 'ireturn'. These instructions result in conditional or
non-conditional changes to the PC. The `call` and `interrupt`
instructions also store state onto the stack to return control back to
the instruction after the calling address (`ret` and `ireturn` perform
these functions). The `CALL` and `RET` instructions together help
implement what is usually called a function call.

Other instructions implement simple arithmetic operations, moving
values between registers, a variety of load instructions and printing
values to the console. All of these instructions are representative of
the types of instructions found in most _real_ CPU architecures. 


#### Go Back and Do It Again

The fetch, decode, execute operations loop until either an exception
occurs or a END instruction is executed. Exceptions can be things
like; attempting to read or write an address in memory that doesn't
exist, attempting to read or write an address that is disallowed by
the current CPU mode, attempting to execute an unknown opcode,
attempting to pop from an empty stack or instruction operands which
are not recognized when decoded. Collectively, these behaviors are
often referred to as an Abnormal End or ABEND. Reaching the END
instruction of a program generally indicates that the program executed
as expected (Alan Turing and his Halting Problem not withstanding).

### Memory and Memory Segmentation

The memory design of our machine is very simple. It consists of an
array of 2000 signed integers, accessed by a address of zero
thru 1999. Values from memory can be read a single address at a
time. Values can be written to memory, a single address at a
time. Attempts to read or write to addresses outside the range of (0,
1999) will result in a hardware error that will cause the CPU to stop
executing the program that caused the error.

The CPU imposes some additional constraints on how memory is accessed,
often referred to as memory segmentation. Our CPU partitions memory
into three regions; USER, TIMER, and INTERRUPT. The first region,
USER, is from 0 to 999 and is accessible to instructions when the CPU
is in USER mode. Attempting to access memory addresses in the TIMER or
INTERRUPT regions while in USER mode results in an error that will
cause the CPU to stop executing the offending program. The TIMER
region of memory is from address 1000 to 1499 and is accessible when
the program in is in SYSTEM mode. The INTERRUPT region is from address
1500 to 1999 and is also accessible in SYSTEM mode. Addresses in the
USER region may also be read and written to when the CPU is in SYSTEM
mode.

#### Stacks and Stack Pointers

A stack is a data structure that has two operations, push and pop, and
whose state is knowing where the top of the stack is. The top of stack
is usually tracked using the Stack Pointer (SP) register, which
contains the address in memory of the top of the stack. Stacks
generally grow "down" from high addresses to low addresses, while code
begins at low addresses and "grows" up in the address space. Stacks
are often called LIFO queues, where LIFO stands for "Last In First
Out". The last value pushed onto the stack is the first value popped
off of the stack.

The push operation is implemented:
1. Decrement the SP by one
2. Store the pushed value at the address "pointed" to by the SP

The pop operation is implemented:
1. Read the value at the address "pointed" to by SP into a register
2. Increment the SP register by one

A stack underflow occurs when a pop operation is attempted on an empty
stack. Stack overflow occurs when the stack "crashes" into some other
region of memory used for another purpose by the program. Our SimpleCPU
memory architecture does not provide any checks for stack overflows. 

Our machine specifies two stacks; a user stack whose base is the top
address of the USER address space (999) and a system stack whose base
is the top address of the INTERRUPT address space (1999). The stacks
grow down from their respective base addresses. When executing in
USER mode, instructions only have access to the user stack. When in
SYSTEM mode (timer and interrupt code), the SYSTEM stack is used.

##### Why Do We Even Want a Stack?

Stacks are a handy way to implement the `CALL` instruction, which
transfers control to an address for a block of instructions which
terminates in a `RET` instruction. The `RET` instruction transfers
control back to the address just after the initiating `CALL`
instruction. The call and return instructions work together using the
stack. When the call instruction is executed, the CPU pushes the
return address onto the stack before changing the PC to address
specified by the call. When the `RET` instruction is executed, the CPU
will pop the address to return to from the stack and set PC to the
new address.

Stacks can also be used to pass arguments from the caller to the
called code. The caller would push values onto the stack before the
call and the called code can rummage thru the stack to find the values
stored there. The called code knows that the top of stack is the
return address and the value at SP+1 is the value pushed by the caller
before the call. This sort of behavior is often called a "calling convention"
since it requires the caller and called code to "agree" on where arguments
are passed and how return values are propagated back to the caller. CPU
archictures which define general purpose registers often use those
to pass function arguments and return values in addition to the stack.

### Instruction Decode Demystified

In both the C and Python implementations, I used a dispatch table to
decode the instruction in the IR register. I could have also used a
really big switch in C and structural pattern matching in Python,
however each of those patterns can be hard to modify and prone to hard
to find errors. The dispatch table pattern makes it easy to add new
instructions and isolates the implementation of each instruction.

In C, I defined an enumeration called `opcode_t` whose integer values
corresponding to the instruction values in the problem
specification. Next, I defined an `instruction_t` structure to
describe an instruction; it's opcode value, whether it needs an
operand, a pointer to the function implementing the instruction, and
other characteristics of the instruction which could be used by the
assembler and disassembler. Finally I built an array of
`instruction_t` structures with statically defined values as shown below:

```C
// instructions.c

instruction_t *alloc_dispatch_table(void)
{
  instruction_t *tbl;

  if (!(tbl = calloc(NUM_OPCODES, sizeof(instruction_t)))) {
    perror("calloc:dispatch_table");
    exit(EXIT_FAILURE);
  }

  tbl[0]  = init_instruction(INVALID_OPCODE, 0, "Invalid", "Invalid opcode.");
  tbl[1]  = init_instruction(LOADV_OPCODE,   1, "LoadV",   "LoadV value: load the value into AC");
  ...										 
  tbl[50] = init_instruction(END_OPCODE,     0, "End",     "End: end execution");
  
  return tbl;
}
```

The decode operation becomes a table lookup, which is a fast and
stable operation with a time complexity of O(1), (no matter how
big the table is, we can always get the item we want in the table
with one operation).

Accessing the dispatch table with the opcode's value provides a
wealth of information about that instruction, including the
"microcode" that implements the functionality of the instruction.

The Python version also implements a dispatch model that uses the
opcode mnemonic to search the CPU class for a method that has the same
name.  Python uses the `dict` data structure for those lookups which
has an average time complexity of O(1) and ends up being just about as
efficient as the C version. Once the method or function is found in
either implmentation, it's invoked to "execute" the instruction.

### Instruction Execution

Once the instruction is decoded, it's time to execute the little
program that implements the instruction. This little program is often
called "microcode" and I've referred to the functions and methods that
implement the SimpleCPU architecture's instruction set as "microcode"
in both C and Python. In C, each entry in the dispatch table has a
pointer to the function that implements the instruction. In Python,
the `CPU` class has methods that implement the instructions as
discussed above.

In C, the `loadv` instruction microcode function looks like:

```C
void load_value(cpu_t *cpu)
{
  /* Load value: Load the value into AC */

  cpu->r_ac = cpu->instruction.operand;
}
```

The instruction loads a value at the address immediately after the
instruction into the AC register. The decode phase has read that value
into a scratch non-architectural register called
`operand`. Non-architectural is a fancy way of saying the the CPU
specification doesn't include it and users of the CPU should not
expect it to be present in other CPUs or even other versions of the
same CPU family.

The `load_value` microcode function is called with a pointer to the `cpu_t`
structure where the registers of the CPU live. The decode phase has already
loaded the registers with the anticipated values and it's up the
microcode to move things around. In this case, the operand is written
to the AC register and the function returns.

The Python implementation of `loadv` is very similar:

```python
class CPU:
    ...
    def loadv(self) -> None:
        """Load the value into the AC register."""
        self.ac = self.operand
```

The `CPU` class implements the registers as attributes and the
instructions with methods. In this case, the `loadv` method is called
with a `self` that is an instance of the CPU class. The function moves
the value in `operand` into the `AC` register and returns, just like
its C function counterpart.

### Program 1: Assembly Language and Machine Code

Loosely speaking, machine code is the raw program information
(instructions and data) that a CPU will execute. Assembly language is
a more human-friendly representation of machine code that can be
"assembled" into machine code using a program called an
"assembler". As I mentioned before, assembly language is specific to a
machine architecture, so programs written in assembly for one CPU will
not execute on another CPU without an activity called "porting".

The following is an assembly language version of program1 found in
the problem specification:
```assembly
;address   mnemonic operand
00000000      loadv 00000072
00000002        put 00000002
00000004      loadv 00000073
00000006        put 00000002
00000008       call 00000011
00000010        end
00000011      loadv 00000010
00000013        put 00000002
00000015        ret
```

The machine code version of program1 looks like:
```assembly
;Address   Opcode
00000000       1
00000001      72
00000002       9
00000003       2
00000004       1
00000005      73
00000006       9
00000007       2
00000008      23
00000009      11
00000010      50
00000011       1
00000012      10
00000013       9
00000014       2
00000015      24
```

The two versions say the same thing, but the assembly language version might be
just a little bit easier to read for the average human. Instead of opcodes, we
use mnemonic labels that stand in for the opcode. At address zero, we use `loadv`
instead of the opcode `1`. Notice in the assembly version the next address after
`loadv` is 00000002 since `loadv` expects an operand in the address directly
after the instruction, 72 in this case. The machine language version shows 72
at address 00000001 but it's unclear if that value is an opcode or an operand.

The assembly language version of the program makes it clearer (for humans).

#### But What Does Program1 Do?

```assembly
00000000      loadv 00000072
00000002        put 00000002
00000004      loadv 00000073
00000006        put 00000002
00000008       call 00000011
00000010        end
00000011      loadv 00000010
00000013        put 00000002
00000015        ret
```

Let's step thru the program line by line:

- 00. The `loadv` instruction loads the value 72 into the AC register
- 02. The `put` instruction writes the value in AC to the console as a character, 'H'
- 04. The `loadv` instruction loads the value 73 into the AC register.
- 06. The `put` instruction writes the value in AC to the console as a character, 'I'
- 08. The `call` instruction pushes 10 onto the stack and sets PC to 11
- 11. The `loadv` instruction loads the value 10 into the AC register
- 13. The `put` instruction writes the value in AC to the console as a chacater, '\n'
- 15. The `ret` instruction pops 10 off the stack and sets the PC to 10
- 10. The `end` instruction causes the CPU stop execution of the program.


We could step thru the machine code version in a similar fashion, but
that sort of punishment is unnecessary!

Running the program with the C version of the simulator would look like this:

```console
$ ./loader -f program1.o
HI
$
```

#### Comparing Assembly to a Higher Level Language

To implement the following C statements for SimpleCPU:

```C
int i = 0;
i++;
```

I would write the following assembly:

```assembly
00 loada 10  // load the value at address 10 (i) into the AC
01 copytox   // copy the AC to the X register
02 incx      // increment X register by 1
03 copyfromx // copy X to the AC register
04 store 10  // store AC to the address 10 (i)
.10          // integer variable i
0            // i initialized to zero
```

The C language abstracts away a lot of things for us, like naming a
memory location 'i' and not having to know it's exact address in
memory. In the assembly version, I had to pick an address to store my
integer variable, so I picked 10. If my program gets more complex,
I'll have to relocate the data to another address higher in the space
and update all the instructions that reference 'i' with the new
address. There is a lot of complexity to manage and it's very easy
for bugs to creep in.

## Conclusion

If you've gotten this far, you have my gratitude and respect! If you've
enjoyed this article or learned something about computers, send me an
email or open an issue to let me know! I had fun writing this article
and these simulators and I hope it sparks a desire to know more about
the inner lives of computers in you. 

Erik - 2 Oct 2023

[0]:https://github.com/JnyJny/SimpleCPU/tree/71465e3292e7c48bd8b6a90135ec2b5b8499e255/docs
