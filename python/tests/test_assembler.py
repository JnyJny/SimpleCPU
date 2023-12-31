"""
"""

import pytest

from simplecpu.asm import Assembler
from simplecpu.dis import Disassembler

from .samples import samples


@pytest.mark.parametrize(
    "source, values",
    [samples["program1"]],
)
def test_assemble_program(source, values) -> None:

    assembler = Assembler(source.splitlines())

    for address, value in enumerate(values):
        assert assembler.memory.read(address) == value
