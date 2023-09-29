"""
"""

from __future__ import annotations

from enum import Enum


class Mode(int, Enum):
    USER: int = 0
    SYSTEM: int = 1


class ProgramLoad(int, Enum):
    USER: int = 0
    TIMER: int = 1000
    INTERRUPT: int = 1500


class StackBase(int, Enum):
    USER: int = 999
    SYSTEM: int = 1999

    @classmethod
    def for_mode(cls, mode: Mode) -> StackBase:
        if mode == Mode.USER:
            return cls.USER
        if mode == Mode.SYSTEM:
            return cls.SYSTEM
        raise ValueError(f"Unknown mode {mode}")


NWORDS: int = 2000

MAGIC: int = 0x6F6A6521
