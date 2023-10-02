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

    for address, line in enumerate([line for line in source.splitlines() if len(line)]):
        value = int(line.split()[0].strip())
        assert assembler.memory.read(address) == value


@pytest.mark.parametrize(
    "source,error",
    [
        (program1, None),
        (sample1, None),
        (sample2, SegmentationFault),
        (sample3, None),
        (sample4, None),
    ],
)
def test_execute_program(source, error) -> None:

    assembler = Assembler(source.splitlines())

    cpu = CPU(assembler.memory)

    if error:
        with pytest.raises(error):
            cpu.start()
    else:
        cpu.start()
