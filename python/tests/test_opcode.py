"""
"""


import pytest

from simplecpu.opcode import Opcode

from simplecpu.exceptions import InvalidOpcodeError


def test_opcode_basic() -> None:

    assert len(Opcode) == 1 + 30 + 1


@pytest.mark.parametrize("opcode", list(Opcode))
def test_opcode_lookup_found(opcode: Opcode) -> None:

    result = Opcode.by_value(opcode.value)

    assert result == opcode
    assert isinstance(result.description, str)
    assert len(result.description)


@pytest.mark.parametrize(
    "opcode", [-10, -5, -1] + list(range(31, 50)) + list(range(51, 60))
)
def test_opcode_lookup_not_found(opcode: int) -> None:

    with pytest.raises(InvalidOpcodeError):
        Opcode.by_value(opcode)


@pytest.mark.parametrize(
    "opcode",
    [
        Opcode.JUMP,
        Opcode.JUMPEQ,
        Opcode.JUMPNE,
        Opcode.CALL,
        Opcode.RET,
        Opcode.INTERRUPT,
        Opcode.IRETURN,
    ],
)
def test_opcode_is_cti(opcode: Opcode) -> None:
    assert opcode.is_cti


@pytest.mark.parametrize(
    "opcode",
    [
        Opcode.LOADV,
        Opcode.LOADA,
        Opcode.LOADI,
        Opcode.LOADX,
        Opcode.LOADY,
        Opcode.LOADSPX,
        Opcode.STORE,
        Opcode.GET,
        Opcode.PUT,
        Opcode.ADDX,
        Opcode.ADDY,
        Opcode.SUBX,
        Opcode.SUBY,
        Opcode.COPYTOX,
        Opcode.COPYFROMX,
        Opcode.COPYTOY,
        Opcode.COPYFROMY,
        Opcode.COPYTOSP,
        Opcode.COPYFROMSP,
        Opcode.INCX,
        Opcode.DECX,
        Opcode.PUSH,
        Opcode.POP,
        Opcode.END,
    ],
)
def test_opcode_is_not_cti(opcode: Opcode) -> None:

    assert not opcode.is_cti


@pytest.mark.parametrize(
    "opcode",
    [
        Opcode.LOADV,
        Opcode.LOADA,
        Opcode.LOADI,
        Opcode.LOADX,
        Opcode.LOADY,
        Opcode.STORE,
        Opcode.PUT,
        Opcode.JUMP,
        Opcode.JUMPEQ,
        Opcode.JUMPNE,
        Opcode.CALL,
    ],
)
def test_opcode_has_operand(opcode: Opcode) -> None:

    assert opcode.has_operand
    assert isinstance(opcode.operand_name, str)


@pytest.mark.parametrize(
    "opcode",
    [
        Opcode.LOADSPX,
        Opcode.GET,
        Opcode.ADDX,
        Opcode.ADDY,
        Opcode.SUBX,
        Opcode.SUBY,
        Opcode.COPYTOX,
        Opcode.COPYFROMX,
        Opcode.COPYTOY,
        Opcode.COPYFROMY,
        Opcode.COPYTOSP,
        Opcode.COPYFROMSP,
        Opcode.RET,
        Opcode.INCX,
        Opcode.DECX,
        Opcode.PUSH,
        Opcode.POP,
        Opcode.INTERRUPT,
        Opcode.IRETURN,
        Opcode.END,
    ],
)
def test_opcode_does_not_have_operand(opcode: Opcode) -> None:

    assert not opcode.has_operand
    assert opcode.operand_name is None
