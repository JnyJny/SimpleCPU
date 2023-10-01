"""
"""

from __future__ import annotations

from enum import Enum

from .exceptions import InvalidOpcodeError


class Instruction(Enum):
    # A tuple of opcode, is_cti, and optional operand_name
    INVALID = (0, False)
    LOADV = (1, False, "value")
    LOADA = (2, False, "address")
    LOADI = (3, False, "address")
    LOADX = (4, False, "address")
    LOADY = (5, False, "address")
    LOADSPX = (6, False)
    STORE = (7, False, "address")
    GET = (8, False)
    PUT = (9, False, "port")
    ADDX = (10, False)
    ADDY = (11, False)
    SUBX = (12, False)
    SUBY = (13, False)
    COPYTOX = (14, False)
    COPYFROMX = (15, False)
    COPYTOY = (16, False)
    COPYFROMY = (17, False)
    COPYTOSP = (18, False)
    COPYFROMSP = (19, False)
    JUMP = (20, True, "address")
    JUMPEQ = (21, True, "address")
    JUMPNE = (22, True, "address")
    CALL = (23, True, "address")
    RET = (24, True)
    INCX = (25, False)
    DECX = (26, False)
    PUSH = (27, False)
    POP = (28, False)
    INTERRUPT = (29, True)
    IRETURN = (30, True)
    END = (50, False)

    @classmethod
    def by_opcode(cls, opcode: int) -> Instruction:
        """Return an Instruction for the given opcode.

        Raises:
        - InvalidOpcodeError
        """
        if opcode == 0:
            raise InvalidOpcodeError(opcode)

        for instruction in cls:
            if instruction.opcode == opcode:
                return instruction

        raise InvalidOpcodeError(opcode)

    def __init__(
        self,
        opcode: int,
        is_cti: bool,
        operand_name: str = None,
    ) -> None:
        self._value_ = opcode
        self._name_ = self._name_.lower()
        self.opcode = opcode
        self.is_cti = is_cti
        self.operand_name = operand_name
        self.has_operand = operand_name is not None