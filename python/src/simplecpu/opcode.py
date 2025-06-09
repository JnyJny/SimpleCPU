"""
"""

from __future__ import annotations

from enum import Enum

from .exceptions import InvalidOpcodeError


class Opcode(Enum):
    # A tuple of opcode, is_cti, description, and optional operand_name
    INVALID = (0, False, "Invalid")
    LOADV = (1, False, "Load value into AC", "value")
    LOADA = (2, False, "Load value from address into AC", "address")
    LOADI = (3, False, "Load value from address at address into AC", "address")
    LOADX = (4, False, "Load value at address + X into AC", "address")
    LOADY = (5, False, "Load value at address + Y into AC", "address")
    LOADSPX = (6, False, "Load value at SP + X into AC")
    STORE = (7, False, "Store AC to address", "address")
    GET = (8, False, "Get a random int from 1 to 100")
    PUT = (9, False, "Write AC to console", "port")
    ADDX = (10, False, "Add X to AC")
    ADDY = (11, False, "Add Y to AC")
    SUBX = (12, False, "Subtract X from AC")
    SUBY = (13, False, "Subtract Y from AC")
    COPYTOX = (14, False, "Copy AC to X")
    COPYFROMX = (15, False, "Copy X to AC")
    COPYTOY = (16, False, "Copy AC to Y")
    COPYFROMY = (17, False, "Copy Y to AC")
    COPYTOSP = (18, False, "Copy AC to SP")
    COPYFROMSP = (19, False, "Copy SP to AC")
    JUMP = (20, True, "Unconditional jump to address", "address")
    JUMPEQ = (21, True, "Jump to address if AC is zero", "address")
    JUMPNE = (22, True, "Jum to address if AC is not zero", "address")
    CALL = (23, True, "Push return value on stack and jump to address", "address")
    RET = (24, True, "Pop return value from stack and jump to address")
    INCX = (25, False, "Increment value in X")
    DECX = (26, False, "Decrement value in Y")
    PUSH = (27, False, "Push AC on to stack.")
    POP = (28, False, "Pop value from stack into AC")
    INTERRUPT = (29, True, "Push SP and return address on to stack and JUMP to 1500")
    IRETURN = (30, True, "Pop SP and return address from stack")
    END = (50, False, "End execution")

    @classmethod
    def by_value(cls, value: int) -> Opcode:
        # this is a hack to get around the broken enum
        # eg. Opcode(1) fails since it expects a tuple

        for opcode in cls:
            if opcode.value == value:
                return opcode

        raise InvalidOpcodeError(value) from None

    def __init__(
        self,
        opcode: int,
        is_cti: bool,
        description: str,
        operand_name: str = None,
    ) -> None:
        self._value_ = opcode
        self._name_ = self._name_.lower()
        self.opcode = opcode
        self.is_cti = is_cti
        self.description = description
        self.operand_name = operand_name
        self.has_operand = operand_name is not None

    def __str__(self) -> str:
        if self.has_operand:
            return f"{self.name:12} {self.operand_name}"
        return self.name

    def __eq__(self, other) -> bool:
        if isinstance(other, int):
            return self.value == other
        return super().__eq__(other)
