""" CPU Simulator
"""

from __future__ import annotations

import functools
import random

from loguru import logger

from .constants import Mode, ProgramLoad, StackBase
from .exceptions import *
from .instruction import Instruction
from .memory import Memory


def instrument(*, registers: bool = True, memory: bool = True, stack: bool = False):
    """Log CPU state prior to the execution of an instruction."""

    def wrapper(func):
        name = func.__name__
        desc = func.__doc__.splitlines()[0]

        @functools.wraps(func)
        def wrapped(*args, **kwargs):
            lgr_ = logger.opt(depth=1)
            cpu, *_ = args
            dump = cpu.dump(registers=registers, memory=memory, stack=stack)
            try:
                lgr_.debug("{:08}{:>16s} {:08} // {}", cpu.pc, name, cpu.operand, desc)
            except TypeError:
                lgr_.debug("{:08}{:>16s} {:8} // {}", cpu.pc, name, "", desc)
            for line in dump.splitlines():
                lgr_.debug(line)
            func(*args, **kwargs)

        return wrapped

    return wrapper


class CPU:
    def __init__(
        self, memory: Memory = None, timer_interval: int = 0, debug: bool = False
    ) -> None:
        self.memory = memory or Memory()
        self.timer_interval = timer_interval
        self.debug: bool = debug
        self.interrupts_enabled: bool = True
        self.fault: bool = False
        self.mode = Mode.USER
        self.ir: int = 0
        self.pc: int = ProgramLoad.for_mode(self.mode).value
        self.sp: int = StackBase.for_mode(self.mode).value
        self.ac: int = 0
        self.x: int = 0
        self.y: int = 0
        self.cycles: int = 0
        self.operand: int = None

    def __str__(self) -> str:
        return self.dump(stack=True)

    def dump(
        self,
        registers: bool = True,
        memory: bool = True,
        stack: bool = False,
    ) -> str:
        """The state of the CPU."""

        lines = []

        status = [
            f"[ m] {self.mode.name:8}",
            f"[ti] {self.timer_interval:08}",
            f"[t?] {str(self.fire_timer):8}",
            f"[i?] {str(self.interrupts_enabled)}",
        ]

        lines.append(" ".join(status))

        base = StackBase.for_mode(self.mode)

        if registers:
            lines.append(
                f"[pc] {self.pc:08} [ir] {self.ir:08} [sp] {self.sp:08} [sb] {base.value:08}"
            )
            lines.append(
                f"[ac] {self.ac:08} [ x] {self.x:08} [ y] {self.y:08} [cy] {self.cycles:08}"
            )

        if stack:
            for sp in range(self.sp, base.value):
                value = self._load(sp)
                lines.append(f"[stack {sp:08}] [{value:08}]")

        if memory:
            lines.extend(str(self.memory).splitlines())

        return "\n".join(lines)

    @property
    def fire_timer(self) -> bool:
        """Return True if the timer_interrupt should be taken."""

        return all(
            [
                self.cycles
                and self.timer_interval
                and self.cycles % self.timer_interval == 0
            ]
        )

    @property
    def instruction(self) -> Instruction:
        """The Instruction for the opcode in the IR register."""
        return Instruction.by_opcode(self.ir)

    @property
    def microcode(self) -> callable:
        """The method that implements the instruction in the IR register.

        Raises:
        - MachineCheck if no method is found for the opcode in IR.
        """
        instruction = Instruction.by_opcode(self.ir)
        try:
            return getattr(self, instruction.name)
        except AttributeError:
            raise MachineCheck(f"Missing microcode for {instruction.name}") from None

    @property
    def user_space(self) -> range:
        """The valid range of addresses accessible in USER mode."""
        try:
            return self._user_space
        except AttributeError:
            pass
        self._user_space = range(ProgramLoad.USER.value, StackBase.USER.value + 1)
        return self._user_space

    def _load(self, address: int) -> int:
        """Load an integer value from Memory at address and return it.

        Raises:
        - SegmentationFault if mode is USER and address is out of bounds.
        """
        if self.mode is Mode.USER and address not in self.user_space:
            raise SegmentationFault(f"load from {address}")
        return self.memory.read(address)

    def _store(self, address: int, value: int) -> None:
        """Store an integer value to Memory at address.

        Raises:
        - SegmentationFault if mode is USER and address is out of bounds.
        """
        if self.mode is Mode.USER and address not in self.user_space:
            raise SegmentationFault(f"store to {address}")
        self.memory.write(address, value)

    def _push(self, value: int) -> None:
        """Push the value onto the current stack.

        Decrement the stack pointer.
        Store value to the top of stack.
        """

        self.sp -= 1
        self._store(self.sp, value)

    def _pop(self) -> int:
        """Pop the value from the top of the stack.

        Check for user mode stack underflow
        Load the value from the top of the stack
        Increment stack pointer
        Check for system mode stack underflow
        Return value
        """
        stackbase = StackBase.for_mode(self.mode)

        if self.sp >= stackbase:
            raise StackUnderflowError(self.sp, self.mode)

        try:
            value = self._load(self.sp)
            self.sp += 1
            return value
        except MemoryRangeError:
            raise StackUnderflowError(self.sp, self.mode) from None

    def start(self) -> None:
        """Start executing the program found at the address ProgramLoad.USER.

        Raises:
        - InvalidOpcodeError
        - InvalidOperandError
        - MemoryRangeError
        - SegmentationFault
        """

        while True:
            try:
                self.step()
            except StopIteration:
                self.cycles += 1
                break

            if self.instruction.is_cti:
                continue

            self.pc += 2 if self.instruction.has_operand else 1

    def step(self) -> None:
        """Execute one instruction at PC

        If the interrupt timer fires switch to SYSTEM mode

        Load the instruction at PC into IR
        Load the operand if the instruction has one
        Execute the microcode for the instruction
        Increment cycles to "retire" the instruction

        Raises:
        - InvalidOpcodeError
        - InvalidOperandError
        - MachineCheck
        - MemoryRangeError
        - SegmentationFault
        """

        self.operand = None

        if self.fire_timer:
            self.interrupt(ProgramLoad.TIMER)

        self.ir = self._load(self.pc)

        if self.instruction.has_operand:
            self.operand = self._load(self.pc + 1)

        self.microcode()

        self.cycles += 1

    @instrument()
    def invalid(self) -> None:
        """Raises InvalidOpcodeError."""
        raise InvalidOpcodeError(f"{self.pc:08d}: {self.ir:08d}")

    @instrument()
    def loadv(self) -> None:
        """Load the value into the AC register."""
        self.ac = self.operand

    @instrument()
    def loada(self) -> None:
        """Load the value at address into the AC register."""
        self.ac = self._load(self.operand)

    @instrument()
    def loadi(self) -> None:
        """Load the value from address at address into the AC register."""
        address = self._load(self.operand)
        self.ac = self._load(address)

    @instrument()
    def loadx(self) -> None:
        """Load value from address + X register into the AC register."""
        self.ac = self._load(self.operand + self.x)

    @instrument()
    def loady(self) -> None:
        """Load value from address + Y register into the AC register."""
        self.ac = self._load(self.operand + self.y)

    @instrument()
    def loadspx(self) -> None:
        """Load the value from address SP+X into the AC register."""
        self.ac = self._load(self.sp + self.x)

    @instrument()
    def store(self) -> None:
        """Store the value in the AC register at address."""
        self._store(self.operand, self.ac)

    @instrument()
    def get(self) -> None:
        """Store a random integer from 1 to 100 into the AC register."""
        self.ac = random.randint(1, 100)

    @instrument()
    def put(self) -> None:
        """Write contents of AC register to the console.

        If port is 1, write AC as an integer
        If port is 2, write AC as a character

        Raises:
        - InvalidOperandError for port not in [1,2]
        """

        logger.info(f"[{self.pc:08}] put {self.ac} -> port {self.operand} ")

        if self.debug:
            return

        if self.operand == 1:
            print(self.ac, end="")
            return

        if self.operand == 2:
            print(chr(self.ac), end="")
            return

        raise InvalidOperandError(f"unknown {port=}")

    @instrument()
    def addx(self) -> None:
        """Add X register into AC register."""
        self.ac += self.x

    @instrument()
    def addy(self) -> None:
        """Add Y register into AC register."""
        self.ac += self.y

    @instrument()
    def subx(self) -> None:
        """Subtract X register into AC register."""
        self.ac -= self.x

    @instrument()
    def suby(self) -> None:
        """Subtract Y register into AC register."""
        self.ac -= self.y

    @instrument()
    def copytox(self) -> None:
        """Copy the AC register to the X register."""
        self.x = self.ac

    @instrument()
    def copyfromx(self) -> None:
        """Copy the X register to the AC register."""
        self.ac = self.x

    @instrument()
    def copytoy(self) -> None:
        """Copy the AC register to the Y register."""
        self.y = self.ac

    @instrument()
    def copyfromy(self) -> None:
        """Copy the Y register to the AC register."""
        self.ac = self.y

    @instrument()
    def copytosp(self) -> None:
        """Copy the AC register to the SP register."""
        self.sp = self.ac

    @instrument()
    def copyfromsp(self) -> None:
        """Copy the SP register to the AC register."""
        self.ac = self.sp

    @instrument()
    def jump(self) -> None:
        """Jump to the address."""
        self.pc = self.operand

    @instrument()
    def jumpeq(self) -> None:
        """Jump to the address of AC register is equal to zero."""
        if self.ac == 0:
            self.pc = self.operand
        else:
            self.pc += 2

    @instrument()
    def jumpne(self) -> None:
        """Jump to the address of AC register is not equal to zero."""
        if self.ac != 0:
            self.pc = self.operand
        else:
            self.pc += 2

    @instrument(stack=True)
    def call(self) -> None:
        """Push return address onto the stack and jump to address."""
        self._push(self.pc + 2)
        self.pc = self.operand

    @instrument(stack=True)
    def ret(self) -> None:
        """Pop return address from the stack and jump to that address."""
        self.pc = self._pop()

    @instrument()
    def incx(self) -> None:
        """Increment the value in the X register by one."""
        self.x += 1

    @instrument()
    def decx(self) -> None:
        """Decrement the value in the X register by one."""
        self.x -= 1

    @instrument(stack=True)
    def push(self) -> None:
        """Push the contents of the AC register onto the stack."""
        self._push(self.ac)

    @instrument(stack=True)
    def pop(self) -> None:
        """Pop the top of the stack into the AC register."""
        self.ac = self._pop()

    @instrument(stack=True)
    def interrupt(self, program_load: ProgramLoad = ProgramLoad.INTERRUPT) -> None:
        """Perform a system call.

        Handles TIMER and INTERRUPT system calls.

        If interrupts are disabled, increment PC and return.

        Switch to SYSTEM mode
        Disable interrupts
        Save SP and PC
        If not a TIMER interrupt, increment PC
        Switch to SYSTEM stack
        Push user SP
        Push user PC
        Set PC to ProgramLoad.INTERRUPT or ProgramLoad.TIMER
        """

        if not self.interrupts_enabled:
            self.pc += 1
            return

        self.mode = Mode.SYSTEM
        self.interrupts_enabled = False

        u_sp = self.sp
        u_pc = self.pc

        if program_load == ProgramLoad.INTERRUPT:
            u_pc += 1

        self.sp = StackBase.for_mode(self.mode).value
        self._push(u_sp)
        self._push(u_pc)

        self.pc = program_load.value

    @instrument(stack=True)
    def ireturn(self) -> None:
        """Return from a system call.

        Pop user PC from the SYSTEM stack
        Pop user SP from the SYSTEM stack
        Enable interrupts
        Switch mode to USER
        """
        self.pc = self._pop()
        self.sp = self._pop()
        self.interrupts_enabled = True
        self.mode = Mode.USER

    @instrument()
    def end(self) -> None:
        """Stop executing instructions.

        Raises StopIteration
        """
        raise StopIteration()
