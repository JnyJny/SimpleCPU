"""
"""

from array import array
from pathlib import Path

from .constants import NWORDS, MAGIC
from .memory import Memory


class ObjectFormatError(Exception):
    def __init__(self, magic: int, path: str | Path) -> None:
        self.magic = magic
        self.path = path

    def __str__(self) -> str:
        return f"{self.path} {self.magic} != {MAGIC}"


class ObjectFile:
    def __init__(self, path: str | Path) -> None:
        self.path = Path(path)

    @property
    def image(self) -> Memory:
        try:
            return self._image
        except AttributeError:
            pass

        nwords = (self.path.stat().st_size // 4) - 1
        magic = array("i")
        data = array("i")
        with self.path.open("rb") as fp:
            magic.fromfile(fp, 1)
            data.fromfile(fp, nwords)

        if magic[0] != MAGIC:
            raise ObjectFormatError(magic[0], self.path)

        self._image = Memory(nwords, data.tolist())
        return self._image

    def write(self, memory: Memory, path: str | Path = None) -> None:

        path = Path(path or self.path)
        magic = array("i", [MAGIC])
        with path.open("wb") as fp:
            magic.tofile(fp)
            memory.words.tofile(fp)
