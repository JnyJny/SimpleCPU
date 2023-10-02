"""
"""

import pytest

from cpusim.cpu import CPU
from cpusim.exceptions import *
from cpusim.asm import Assembler


from .program1 import program1
from .samples import sample1, sample2, sample3, sample4


@pytest.mark.parametrize(
    "source",
    [
        program1,
    ],
)
def test_assemble_program(source) -> None:

    assembler = Assembler(source.splitlines())
    assembler.parse()

    for address, line in enumerate([line for line in source.splitlines() if len(line)]):
        value = int(line.split()[0].strip())
        assert assembler.memory.read(address) == value


@pytest.mark.parametrize(
    "source",
    [
        program1,
        sample1,
        sample2,
        sample3,
        sample4,
    ],
)
def test_execute_program(source) -> None:
    assembler = Assembler(source.splitlines())

    assembler.parse()

    cpu = CPU(assembler.memory)

    cpu.start()
