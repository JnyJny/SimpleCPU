"""
"""

from enum import Enum


class InvalidOpcodeError(Exception):
    def __init__(self, address: int, opcode: int) -> None:
        self.address = address
        self.opcode = opcode

    def __str__(self) -> str:
        return f"Invalid Opcode {self.opcode:02d} at {self.address:08d}"


class Opcode(int, Enum):
    INVALID = 0
    LOADV = 1
    LOADA = 2
    LOADI = 3
    LOADX = 4
    LOADY = 5
    LOADSPX = 6
    STORE = 7
    GET = 8
    PUT = 9
    ADDX = 10
    ADDY = 11
    SUBX = 12
    SUBY = 13
    COPYTOX = 14
    COPYFROMX = 15
    COPYTOY = 16
    COPYFROMY = 17
    COPYTOSP = 18
    COPYFROMSP = 19
    JUMP = 20
    JUMPEQ = 21
    JUMPNE = 22
    CALL = 23
    RET = 24
    INCX = 25
    DECX = 26
    PUSH = 27
    POP = 28
    INTERRUPT = 29
    IRETURN = 30
    END = 50

    @property
    def has_operand(self) -> bool:
        return self in [
            Opcode.LOADV,
            Opcode.LOADA,
            Opcode.LOADI,
            Opcode.LOADX,
            Opcode.LOADY,
            Opcode.STORE,
            Opcode.PUT,
            Opcode.JUMP,
            Opcode.JUMPEQ,
            Opcode.JUMPNE,
            Opcode.CALL,
        ]

    @property
    def is_cti(self) -> bool:
        if self in range(Opcode.JUMP, Opcode.RET + 1):
            return True
        return self in [Opcode.INTERRUPT, Opcode.IRETURN]
