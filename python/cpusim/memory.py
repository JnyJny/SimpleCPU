"""
"""

from __future__ import annotations

from array import array
from pathlib import Path
from struct import unpack

from loguru import logger

from .constants import NWORDS, ProgramLoad


class MemoryRangeError(Exception):
    def __init__(self, address: int, bounds: range) -> None:
        self.address = address
        self.bounds = bounds

    def __str__(self) -> str:
        return f"{self.__class__.__name__} {self.address} not in {self.bounds}"


class Memory:
    def __init__(self, nwords: int = NWORDS, initializer: list[int] = None) -> None:
        """ """
        self.nwords = nwords
        self.initializer = initializer

        if self.nwords <= 0:
            raise ValueError(
                f"Non-positive memory size is not supported: {self.nwords}"
            )

        if self.initializer and len(self.initializer) != self.nwords:
            raise ValueError(
                f"Initializer mismatch: {self.nwords} != {len(self.initializer)}"
            )

        logger.debug(repr(self))

    def __repr__(self) -> str:
        return f"{self.__class__.__name__}(nwords={self.nwords})"

    def __str__(self) -> str:
        lines = ["           " + " ".join([f"[{n:06}]" for n in range(0, 10)])]
        for n in range(0, self.nwords // 10):
            base = n * 10
            if not sum(self.words[base : base + 10]):
                continue
            lines.append(
                f"[{n * 10:08d}] "
                + " ".join(map(lambda v: f"{v:08}", self.words[base : base + 10]))
            )
        return "\n".join(lines)

    def __eq__(self, other: Memory) -> bool:
        return all([self.nwords == other.nwords, self.words == other.words])

    def __bytes__(self) -> bytes:
        return bytes(self.words)

    @property
    def words(self) -> array:
        try:
            return self._words
        except AttributeError:
            pass
        self._words = array("i", self.initializer or [0] * self.nwords)
        return self._words

    @property
    def bounds(self) -> range:
        return range(ProgramLoad.USER, ProgramLoad.USER + self.nwords)

    def dump(self) -> str:

        lines = ["           " + " ".join([f"[{n:06}]" for n in range(0, 10)])]
        for n in range(0, self.nwords // 10):
            base = n * 10
            lines.append(
                f"[{n * 10:08d}] "
                + " ".join(map(lambda v: f"{v:08}", self.words[base : base + 10]))
            )
        return "\n".join(lines)

    def read(self, address: int) -> int:
        """ """
        logger.debug(f"read request {address=}")
        if address not in self.bounds:
            outofbounds = MemoryRangeError(address, self.bounds)
            logger.error(str(outofbounds))
            raise outofbounds
        return self.words[address]

    def write(self, address: int, value: int) -> None:
        """ """
        logger.debug(f"write request {address=} {value=}")
        if address not in self.bounds:
            outofbounds = MemoryRangeError(address, self.bounds)
            logger.error(str(outofbounds))
            raise outofbounds
        self.words[address] = value
