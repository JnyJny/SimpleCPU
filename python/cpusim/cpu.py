""" CPU Simulator
"""

from __future__ import annotations

import functools
import random

from loguru import logger

from .constants import Mode, ProgramLoad, StackBase
from .memory import Memory, MemoryRangeError
from .opcode import Opcode, InvalidOpcodeError


class SegmentationFault(Exception):
    pass


class InvalidOperandError(Exception):
    pass


class StackUnderflowError(Exception):
    pass


class MachineCheck(Exception):
    pass


def instrument(*, registers: bool = True, memory: bool = True, stack: bool = False):
    def wrapper(func):
        name = func.__name__
        desc = func.__doc__

        @functools.wraps(func)
        def wrapped(*args, **kwargs):
            logger_ = logger.opt(depth=1)
            cpu, *_ = args

            opcode = cpu.opcode
            try:
                logger_.debug(
                    "{:08}{:>16s} {:08} // {}", cpu.pc, name, cpu.operand, desc
                )
            except TypeError:
                logger_.debug("{:08}{:>16s} {:8} // {}", cpu.pc, name, "", desc)

            for line in cpu.dump(
                registers=registers, memory=memory, stack=stack
            ).splitlines():
                logger_.debug(line)
            return func(*args, **kwargs)

        return wrapped

    return wrapper


class CPU:
    def __init__(
        self, memory: Memory, timer_interval: int = 0, debug: bool = False
    ) -> None:
        self.memory = memory
        self.timer_interval = timer_interval
        self.debug: bool = debug
        self.interrupts_enabled: bool = True
        self.fault: bool = False
        self.mode = Mode.USER
        self.ir: int = 0
        self.pc: int = ProgramLoad.USER.value
        self.sp: int = StackBase.for_mode(self.mode).value
        self.ac: int = 0
        self.x: int = 0
        self.y: int = 0
        self.cycles: int = 0
        self.operand: int = None

    def __str__(self) -> str:
        return self.dump()

    def log_state(self) -> None:
        for line in str(self).splitlines():
            logger.debug(line)

    def dump(
        self, registers: bool = True, memory: bool = True, stack: bool = False
    ) -> str:
        lines = []
        if registers:
            fmt = (
                "[pc] {pc:08} [ir] {ir:08} [sp] {sp:08}\n"
                "[ac] {ac:08} [ x] {x:08} [ y] {y:08}\n"
                "[cy] {cycles:08}"
            )
            lines.append(fmt.format_map(self.registers))

        if stack:
            base = StackBase.for_mode(self.mode)
            for sp in range(self.sp, base.value):
                value = self._load(sp)
                lines.append(f"[stack {sp:08}] [{value:08}]")

        if memory:
            lines.extend(str(self.memory).splitlines())

        return "\n".join(lines)

    @property
    def registers(self) -> dict[str, int]:
        return {
            "pc": self.pc,
            "ir": self.ir,
            "sp": self.sp,
            "ac": self.ac,
            "x": self.x,
            "y": self.y,
            "cycles": self.cycles,
            "mode": self.mode,
        }

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
    def opcode(self) -> Opcode:
        return Opcode(self.ir)

    @property
    def user_space(self) -> range:
        try:
            return self._user_space
        except AttributeError:
            pass
        self._user_space = range(ProgramLoad.USER, int(StackBase.USER) + 1)
        return self._user_space

    def _load(self, address: int) -> int:
        if self.mode is Mode.USER and address not in self.user_space:
            raise SegmentationFault(address)
        return self.memory.read(address)

    def _store(self, address: int, value: int) -> None:
        if self.mode is Mode.USER and address not in self.user_space:
            raise SegmentationFault(address)
        self.memory.write(address, value)

    def _push(self, value: int) -> None:
        self.sp -= 1
        self._store(self.sp, value)

    def _pop(self) -> int:

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
        """Start executing the program found at the address ProgramLoad.USER."""

        while True:
            try:
                self.step()
            except StopIteration:
                break

    def step(self) -> None:
        """Execute one instruction at PC"""
        self.operand = None

        if self.fire_timer:
            self.interrupt(ProgramLoad.TIMER)

        self.ir = self._load(self.pc)

        try:
            opcode = Opcode(self.ir)
        except ValueError:
            raise InvalidOpcodeError(self.pc, self.ir) from None

        if opcode.has_operand:
            self.operand = self._load(self.pc + 1)

        try:
            microcode = getattr(self, opcode.name.lower())
        except AttributeError:
            raise MachineCheck(f"Missing microcode for {opcode.name.lower()}")
        try:
            microcode()
        except StopIteration:
            self.cycles += 1
            raise

        self.cycles += 1

        if opcode.is_cti:
            return

        self.pc += 2 if opcode.has_operand else 1

    @instrument()
    def invalid(self) -> None:
        raise InvalidOpcodeError(self.pc, self.ir)

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
        """Write contents of AC register to the console as an int (1) or character(2)."""

        logger.info(f"[{self.pc:08}] put {self.ac} -> port {self.operand} ")

        if self.debug:
            return

        if self.operand == 1:
            print(self.ac, end="")
            return

        if self.operand == 2:
            print(chr(self.ac), end="")
            return

        raise InvalidOperandError(port)

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
        """Perform a system call."""

        if not self.interrupts_enabled:
            self.pc += 1
            return

        self.mode = Mode.SYSTEM
        self.interrupts_enabled = False

        u_sp = self.sp
        u_pc = self.pc + 1

        self.sp = StackBase.SYSTEM.value
        self._push(u_sp)
        self._push(u_pc)

        self.pc = program_load.value

    @instrument(stack=True)
    def ireturn(self) -> None:
        """Return from a system call."""
        self.pc = self._pop()
        self.sp = self._pop()
        self.interrupts_enabled = True
        self.mode = Mode.USER

    @instrument()
    def end(self) -> None:
        """Stop executing instructions."""
        raise StopIteration()
