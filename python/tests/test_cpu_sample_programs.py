"""
"""

import pytest

from cpusim.cpu import CPU
from cpusim.exceptions import *
from cpusim.asm import Assembler

from .samples import samples


@pytest.mark.parametrize(
    "source,expected_exception",
    [
        (samples["program1"][0], None),
        (samples["sample1"][0], None),
        (samples["sample2"][0], SegmentationFault),
        (samples["sample3"][0], None),
        (samples["sample4"][0], None),
    ],
)
def test_execute_program(source, expected_exception) -> None:

    assembler = Assembler(source.splitlines())

    cpu = CPU(assembler.memory)

    if expected_exception:
        with pytest.raises(expected_exception):
            cpu.start()
    else:
        cpu.start()
