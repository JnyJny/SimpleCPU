"""
"""

from __future__ import annotations

from array import array
from pathlib import Path
from struct import unpack

from loguru import logger

from .constants import NWORDS, ProgramLoad, MAGIC
from .exceptions import MemoryRangeError, ObjectFormatError


class Memory:
    @classmethod
    def from_file(cls, path: str | Path) -> Memory:
        """Read a memory image from a file and return an initalized Memory."""
        buf = Path(path).read_bytes()
        magic = int.from_bytes(buf[:4])
        return cls.from_bytes(magic, buf[4:])

    @classmethod
    def from_bytes(cls, magic: int, buffer: bytes) -> Memory:
        """Create a Memory object loaded with the contents of the bytes buffer."""

        if magic != MAGIC:
            raise ObjectFormatError(f"Bad magic: {magic} != {MAGIC}")

        data = array("i")
        data.frombytes(buffer)

        return cls(len(data), initializer=data.tolist())

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
        return f"{self.__class__.__name__}(nwords={self.nwords}, initializer=...)"

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
        """A bytes object prefaced with MAGIC and the contents of memory."""
        return MAGIC.to_bytes(4) + self.words.tobytes()

    def __len__(self) -> int:
        return len(self.words)

    @property
    def words(self) -> array:
        """An array of integers."""
        try:
            return self._words
        except AttributeError:
            pass
        self._words = array("i", self.initializer or [0] * self.nwords)
        return self._words

    @property
    def bounds(self) -> range:
        """Returns a range object describing the bounds of this memory."""
        return range(ProgramLoad.USER, ProgramLoad.USER + self.nwords)

    def dump(self) -> str:
        """Return a string representation of data held by this memory."""

        lines = ["           " + " ".join([f"[{n:06}]" for n in range(0, 10)])]
        for n in range(0, self.nwords // 10):
            base = n * 10
            lines.append(
                f"[{n * 10:08d}] "
                + " ".join(map(lambda v: f"{v:08}", self.words[base : base + 10]))
            )
        return "\n".join(lines)

    def read(self, address: int) -> int:
        """Read an integer at the given address and return it to the caller.

        Raises:
        - MemoryRangeError
        """
        if address not in self.bounds:
            outofbounds = MemoryRangeError(f"{address} not in {self.bounds}")
            logger.error(str(outofbounds))
            raise outofbounds

        value = self.words[address]

        logger.debug(f"read request {address=} -> {value=}")

        return value

    def write(self, address: int, value: int) -> None:
        """Write an integer value to the given address.

        Raises:
        - MemoryRangeError
        """
        logger.debug(f"write request {address=} {value=}")

        if address not in self.bounds:
            outofbounds = MemoryRangeError(f"{address} not in {self.bounds}")
            logger.error(str(outofbounds))
            raise outofbounds

        self.words[address] = value

    def save(self, path: str | Path) -> None:
        """Save the contents of memory to the specified path."""
        Path(path).write_bytes(bytes(self))
