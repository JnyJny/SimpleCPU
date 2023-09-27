"""
"""


from io import TextIOWrapper
from pathlib import Path

from loguru import logger

from .constants import ProgramLoad
from .memory import Memory


class Loader:
    def __init__(self, fileobj: TextIOWrapper, debug: bool = False) -> None:
        self.fileobj = fileobj

    def initialize(self, memory: Memory) -> None:

        address = ProgramLoad.USER.value

        for line, text in enumerate(self.fileobj.readlines()):
            logger.debug(f"[{line=:05} {address=:08}] {text=}")
            text = text.strip()
            if not text:
                continue

            if text.startswith("."):
                address = int(text.lstrip("."))
                continue

            try:
                values = text.split()
                memory.write(address, int(values[0]))
            except Exception as error:
                logger.debug(f"{text=} {error}")
            address += 1
