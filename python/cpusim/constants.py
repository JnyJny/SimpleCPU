"""
"""

from enum import Enum


class Mode(int, Enum):
    USER: int = 0
    SYSTEM: int = 1


class ProgramLoad(int, Enum):
    USER = 0
    TIMER = 1000
    INTERRUPT = 1500


class StackBase(int, Enum):
    USER: int = 999
    SYSTEM: int = 1999


NWORDS: int = 2000

MAGIC: int = 0x6F6A6521
