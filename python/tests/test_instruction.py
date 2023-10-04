"""
"""

import pytest

from simplecpu.instruction import Instruction
from simplecpu.opcode import Opcode
from simplecpu.exceptions import InvalidOpcodeError


@pytest.mark.parametrize("opcode", list(Opcode))
def test_instruction_with_valid_opcodes(opcode: Opcode) -> None:

    instruction_o = Instruction(opcode)

    assert instruction_o.opcode == opcode

    instruction_v = Instruction(opcode.value)

    assert instruction_v.opcode == opcode


@pytest.mark.parametrize(
    "opcode",
    [
        -20,
        -10,
        -1,
        31,
        32,
        33,
        34,
        34,
        35,
        36,
        37,
        38,
        39,
        40,
        41,
        42,
        43,
        44,
        45,
        46,
        47,
        48,
        49,
        51,
    ],
)
def test_instruction_with_invalid_opcodes(opcode: int) -> None:

    with pytest.raises(InvalidOpcodeError):
        Instruction(opcode)


@pytest.mark.parametrize(
    "operand",
    list(range(0, 8192, 16)),
)
def test_instruction_with_operand(operand: int) -> None:

    instruction = Instruction(Opcode.LOADV, operand=operand)

    assert instruction.operand == operand


@pytest.mark.parametrize("address", list(range(0, 2000, 10)))
def test_instruction_with_address(address: int) -> None:

    instruction = Instruction(Opcode.LOADV, address)

    assert instruction.address == address


@pytest.mark.parametrize(
    "address,operand",
    [(a, o) for a, o in zip(range(0, 1999, 10), range(1999, -1, -10))],
)
def test_instruction_with_address_and_operand(address: int, operand: int) -> None:

    instruction = Instruction(Opcode.LOADV, address, operand)

    assert instruction.address == address
    assert instruction.operand == operand
