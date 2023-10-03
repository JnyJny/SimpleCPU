"""
"""

import pytest

from simplecpu.memory import Memory, MemoryRangeError
from simplecpu.constants import NWORDS, ProgramLoad

MAX_UNSIGNED_INT = 0x7FFFFFFF


@pytest.fixture
def memory_zero() -> Memory:
    return Memory()


@pytest.fixture
def memory_nonzero() -> Memory:
    return Memory(initializer=[MAX_UNSIGNED_INT] * NWORDS)


def test_memory_basic(memory_zero, memory_nonzero) -> None:

    memory = Memory()

    assert memory
    assert isinstance(memory, Memory)

    assert memory.nwords == NWORDS
    assert len(memory.bounds) == memory.nwords

    assert sum(memory.words) == 0

    assert memory == memory_zero
    assert memory != memory_nonzero

    for address in memory.bounds:
        assert memory.words[address] == 0
        assert memory.read(address) == 0

    assert isinstance(bytes(memory), bytes)


@pytest.mark.parametrize(
    "value",
    [
        0,
        1,
        0xFF,
        0xF0F0,
        MAX_UNSIGNED_INT,
    ],
)
def test_memory_io(value) -> None:

    memory = Memory()

    for address in memory.bounds:
        assert memory.words[address] == 0
        memory.write(address, value)
        assert memory.words[address] == value
        assert memory.read(address) == value


@pytest.mark.parametrize("nwords", [1, 100, NWORDS, NWORDS * 2])
def test_memory_init_nwords(nwords) -> None:

    memory = Memory(nwords)

    assert memory.nwords == nwords
    assert len(memory.bounds) == nwords

    lo_oob = memory.bounds.start - 1
    hi_oob = memory.bounds.stop + 1

    for address in [lo_oob, hi_oob]:
        with pytest.raises(MemoryRangeError):
            memory.read(address)

        with pytest.raises(MemoryRangeError):
            memory.write(address, 0)


@pytest.mark.parametrize(
    "nwords, initializer",
    [
        (NWORDS, [1] * NWORDS),
        (100, [0xFF] * 100),
        (100, list(range(0, 200, 2))),
    ],
)
def test_memory_init_initializer(nwords, initializer) -> None:

    memory = Memory(nwords, initializer=initializer)

    for n, address in enumerate(memory.bounds):
        assert memory.words[address] == initializer[n]


@pytest.mark.parametrize("nwords", [0, -1])
def test_memory_init_bogus_nwords(nwords) -> None:

    with pytest.raises(ValueError):
        memory = Memory(nwords)


@pytest.mark.parametrize(
    "nwords, initializer",
    [
        (100, list(range(0, 100, 2))),
    ],
)
def test_memory_init_bogus_initializer(nwords, initializer) -> None:

    with pytest.raises(ValueError):
        memory = Memory(nwords, initializer=initializer)
