"""
"""

from __future__ import annotations

from dataclasses import dataclass

from .opcode import Opcode


@dataclass
class Instruction:
    opcode: Opcode
    address: int = None
    operand: int = None

    def __post_init__(self) -> None:

        if isinstance(self.opcode, int):
            self.opcode = Opcode.by_value(self.opcode)

    def __str__(self) -> str:
        words = []
        if self.address is not None:
            words.append(f"{self.address:08d}")
        words.append(f"{self.opcode.name:>12}")
        if self.has_operand:
            if self.operand is not None:
                words.append(f"{self.operand:08d}")
            else:
                words.append(f"{self.opcode.operand_name:8}")
        else:
            words.append(" " * 8)
        words.append(f"; {self.description}")
        return " ".join(words)

    def __eq__(self, other) -> bool:
        return all(
            [
                self.opcode == other.opcode,
                self.address == other.address,
                self.operand == other.operand,
            ],
        )

    def __lt__(self, other) -> bool:
        return self.address < other.address

    @property
    def name(self) -> str:
        return self.opcode.name

    @property
    def is_cti(self) -> bool:
        return self.opcode.is_cti

    @property
    def has_operand(self) -> bool:
        return self.opcode.has_operand

    @property
    def description(self) -> str:
        return self.opcode.description
