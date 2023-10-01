"""
"""

import string

import pytest

from cpusim.cpu import CPU
from cpusim.constants import Mode, ProgramLoad, StackBase
from cpusim.exceptions import *
from cpusim.instruction import Instruction
from cpusim.memory import Memory


@pytest.fixture()
def memory() -> Memory:
    return Memory()


@pytest.fixture()
def cpu(memory) -> CPU:
    return CPU(memory)


def test_cpu_basic(memory) -> None:

    cpu = CPU(memory)

    assert cpu.memory == memory
    assert cpu.timer_interval == 0
    assert cpu.debug == False
    assert cpu.interrupts_enabled == True
    assert cpu.mode == Mode.USER

    assert cpu.ir == 0
    assert cpu.pc == ProgramLoad.USER.value
    assert cpu.sp == StackBase.for_mode(cpu.mode)
    assert cpu.ac == 0
    assert cpu.x == 0
    assert cpu.y == 0
    assert cpu.cycles == 0
    assert cpu.operand is None


@pytest.mark.parametrize("instruction", list(Instruction))
def test_cpu_instructions_methods_present(instruction, cpu) -> None:
    """Check that CPU implements methods for all the instructions."""
    assert getattr(cpu, instruction.name)


def test_cpu_fire_timer_true(cpu) -> None:
    """Test CPU.fire_timer attribute is True"""

    cpu.timer_interval = 100
    cpu.cycles = 100

    assert cpu.fire_timer == True


def test_cpu_fire_timer_false(cpu) -> None:
    """Test CPU.fire_timer attribute is False"""
    cpu.timer_interval = 100
    cpu.cycles = 101
    assert cpu.fire_timer == False


@pytest.mark.parametrize("instruction", list(Instruction))
def test_cpu_instruction_attribute(instruction, cpu) -> None:
    """Dumb test. I feel bad."""
    cpu.ir = instruction.opcode
    assert cpu.instruction == instruction


def test_cpu_user_space_attribute(cpu) -> None:
    """Tests that user_space attribute is a range object."""
    assert isinstance(cpu.user_space, range)


def test_cpu__load_method_good(cpu) -> None:
    """Tests _load method with all addresses in user space."""
    for address in cpu.user_space:
        value = cpu._load(address)
        assert value == 0


def test_cpu__load_method_user_space_oob(cpu) -> None:
    """Tests _load method with two addresses not in user space."""
    for address in [cpu.user_space.start - 1, cpu.user_space.stop + 1]:
        with pytest.raises(SegmentationFault):
            cpu._load(address)


def test_cpu__store_method(cpu) -> None:
    """Test the _store method writing to user space in USER mode."""
    for address in cpu.user_space:
        cpu._store(address, 1)
        assert cpu.memory.words[address] == 1


def test_cpu__store_method_user_space_oob(cpu) -> None:
    """Test the _store  method writing to non-user space address in USER mode."""
    for address_oob in cpu.user_space:
        with pytest.raises(SegmentationFault):
            cpu._store(address_oob + 1000, 1)
        assert cpu.memory.words[address_oob] != 1


def test_cpu__push_method_user_mode(cpu) -> None:

    base = StackBase.USER.value

    assert cpu.sp == base

    cpu._push(1)

    assert cpu.sp == base - 1
    assert cpu._load(cpu.sp) == 1


def test_cpu__push_method_system_mode(cpu) -> None:

    cpu.mode = Mode.SYSTEM
    cpu.sp = StackBase.SYSTEM.value

    base = StackBase.SYSTEM.value

    assert cpu.sp == base

    cpu._push(1)

    assert cpu.sp == base - 1
    assert cpu._load(cpu.sp) == 1


def test_cpu__pop_method_user_mode(cpu) -> None:

    assert cpu.sp == StackBase.for_mode(cpu.mode)

    cpu._push(0xFF)

    assert cpu.sp < StackBase.for_mode(cpu.mode)

    value = cpu._pop()

    assert cpu.sp == StackBase.for_mode(cpu.mode)
    assert value == 0xFF


def test_cpu__pop_method_system_mode(cpu) -> None:

    cpu.mode = Mode.SYSTEM
    cpu.sp = StackBase.for_mode(cpu.mode)

    cpu._push(0xFF)

    assert cpu.sp < StackBase.for_mode(cpu.mode)

    value = cpu._pop()

    assert cpu.sp == StackBase.for_mode(cpu.mode)
    assert value == 0xFF


def test_cpu__pop_method_user_mode_underflow(cpu) -> None:

    assert cpu.mode == Mode.USER
    assert cpu.sp == StackBase.USER.value

    with pytest.raises(StackUnderflowError):
        cpu._pop()


def test_cpu__pop_method_system_mode_underflow(cpu) -> None:

    cpu.mode = Mode.SYSTEM
    cpu.sp = StackBase.for_mode(cpu.mode)

    assert cpu.sp == StackBase.SYSTEM.value

    with pytest.raises(StackUnderflowError):
        cpu._pop()


def test_cpu_start_method(cpu) -> None:
    cpu.memory.words[0] = Instruction.END.opcode

    assert cpu.timer_interval == 0
    assert cpu.fire_timer == False

    cpu.start()

    assert cpu.mode == Mode.USER
    assert cpu.debug == False
    assert cpu.pc == 0
    assert cpu.ir == Instruction.END.opcode
    assert cpu.sp == StackBase.for_mode(cpu.mode)
    assert cpu.ac == 0
    assert cpu.x == 0
    assert cpu.y == 0
    assert cpu.cycles == 1
    assert cpu.operand == None


def test_cpu_microcode_invalid(cpu) -> None:

    cpu.memory.words[0] = Instruction.INVALID.opcode

    with pytest.raises(InvalidOpcodeError):
        cpu.step()  # invalid


def test_cpu_microcode_loadv(cpu) -> None:
    cpu.memory.words[0] = Instruction.LOADV.opcode
    cpu.memory.words[1] = 1

    cpu.step()  # loadv

    assert cpu.ir == Instruction.LOADV
    assert cpu.ac == 1
    assert cpu.operand == 1


def test_cpu_microcode_loada(cpu) -> None:
    cpu.memory.words[0] = Instruction.LOADA.opcode
    cpu.memory.words[1] = 2
    cpu.memory.words[2] = 0xFF

    cpu.step()  # loada

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Instruction.LOADA
    assert cpu.ac == 0xFF
    assert cpu.operand == 2


def test_cpu_microcode_loadi(cpu) -> None:
    cpu.memory.words[0] = Instruction.LOADI.opcode
    cpu.memory.words[1] = 3
    cpu.memory.words[2] = 0xFF
    cpu.memory.words[3] = 2

    cpu.step()  # loadi

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Instruction.LOADI
    assert cpu.ac == 0xFF
    assert cpu.operand == 3


def test_cpu_microcode_loadx(cpu) -> None:
    cpu.memory.words[0] = Instruction.LOADX.opcode
    cpu.memory.words[1] = 99
    cpu.memory.words[100] = 0xFF

    cpu.x = 1

    cpu.step()  # loadx

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Instruction.LOADX
    assert cpu.ac == 0xFF
    assert cpu.operand == 99


def test_cpu_microcode_loady(cpu) -> None:
    cpu.memory.words[0] = Instruction.LOADY.opcode
    cpu.memory.words[1] = 99
    cpu.memory.words[100] = 0xFF

    cpu.y = 1

    cpu.step()  # loady

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Instruction.LOADY
    assert cpu.ac == 0xFF
    assert cpu.operand == 99


def test_cpu_microcode_loadspx(cpu) -> None:

    cpu.memory.words[0] = Instruction.LOADSPX.opcode
    cpu.memory.words[cpu.sp - 100] = 0xFF
    cpu.x = -100

    cpu.step()  # loadspx

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Instruction.LOADSPX
    assert cpu.ac == 0xFF


def test_cpu_microcode_store(cpu) -> None:
    cpu.memory.words[0] = Instruction.STORE.opcode
    cpu.memory.words[1] = 10
    cpu.ac = 0xFF

    cpu.step()  # store

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Instruction.STORE
    assert cpu.operand == 10
    assert cpu.memory.words[10] == 0xFF


def test_cpu_microcode_get(cpu) -> None:
    cpu.memory.words[0] = Instruction.GET.opcode

    assert cpu.ac == 0

    cpu.step()  # get

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Instruction.GET
    assert cpu.ac in range(1, 101)


@pytest.mark.parametrize("char", string.printable)
def test_cpu_microcode_put_port_1(char, cpu, capsys) -> None:

    cpu.memory.words[0] = Instruction.PUT.opcode
    cpu.memory.words[1] = 1

    cpu.ac = ord(char)

    cpu.step()  # put

    captured = capsys.readouterr()

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Instruction.PUT
    assert cpu.operand == 1
    assert captured.out == str(cpu.ac)


@pytest.mark.parametrize("char", string.printable)
def test_cpu_microcode_put_port_2(char, cpu, capsys) -> None:
    cpu.memory.words[0] = Instruction.PUT.opcode
    cpu.memory.words[1] = 2

    cpu.ac = ord(char)

    cpu.step()  # put

    captured = capsys.readouterr()

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Instruction.PUT
    assert cpu.operand == 2

    assert captured.out == chr(cpu.ac)


@pytest.mark.parametrize("ac, x", [(ac, x) for ac, x in zip(range(0, 10), (0, 10))])
def test_cpu_microcode_addx(ac, x, cpu) -> None:
    cpu.memory.words[0] = Instruction.ADDX.opcode
    cpu.ac = ac
    cpu.x = x

    cpu.step()  # addx

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Instruction.ADDX
    assert cpu.ac == ac + x


@pytest.mark.parametrize("ac, y", [(ac, y) for ac, y in zip(range(0, 10), (0, 10))])
def test_cpu_microcode_addy(ac, y, cpu) -> None:
    cpu.memory.words[0] = Instruction.ADDY.opcode
    cpu.ac = ac
    cpu.y = y

    cpu.step()  # addy

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Instruction.ADDY
    assert cpu.ac == ac + y


@pytest.mark.parametrize("ac, x", [(ac, x) for ac, x in zip(range(0, 10), (0, 10))])
def test_cpu_microcode_subx(ac, x, cpu) -> None:
    cpu.memory.words[0] = Instruction.SUBX.opcode

    cpu.ac = ac
    cpu.x = x

    cpu.step()  # subx

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Instruction.SUBX
    assert cpu.ac == ac - x


@pytest.mark.parametrize("ac, y", [(ac, y) for ac, y in zip(range(0, 10), (0, 10))])
def test_cpu_microcode_suby(ac, y, cpu) -> None:
    cpu.memory.words[0] = Instruction.SUBY.opcode

    cpu.ac = ac
    cpu.y = y

    cpu.step()  # suby

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Instruction.SUBY
    assert cpu.ac == ac - y


@pytest.mark.parametrize("ac", list(range(1, 10)))
def test_cpu_microcode_copytox(ac, cpu) -> None:
    cpu.memory.words[0] = Instruction.COPYTOX.opcode
    cpu.ac = ac

    assert cpu.x == 0

    cpu.step()  # copytox

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Instruction.COPYTOX
    assert cpu.x == cpu.ac


@pytest.mark.parametrize("x", list(range(1, 10)))
def test_cpu_microcode_copyfromx(x, cpu) -> None:
    cpu.memory.words[0] = Instruction.COPYFROMX.value

    cpu.x = x
    assert cpu.ac == 0

    cpu.step()  # copyfromx

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Instruction.COPYFROMX
    assert cpu.ac == cpu.x


@pytest.mark.parametrize("ac", list(range(1, 10)))
def test_cpu_microcode_copytoy(ac, cpu) -> None:
    cpu.memory.words[0] = Instruction.COPYTOY.opcode

    cpu.ac = ac

    assert cpu.y == 0

    cpu.step()  # copytoy

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Instruction.COPYTOY
    assert cpu.y == cpu.ac


@pytest.mark.parametrize("y", list(range(1, 10)))
def test_cpu_microcode_copyfromx(y, cpu) -> None:
    cpu.memory.words[0] = Instruction.COPYFROMY.opcode

    cpu.y = y

    assert cpu.ac == 0

    cpu.step()  # copyfromy

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Instruction.COPYFROMY
    assert cpu.ac == cpu.y


@pytest.mark.parametrize("ac", list(range(1, 10)))
def test_cpu_microcode_copytosp(ac, cpu) -> None:
    cpu.memory.words[0] = Instruction.COPYTOSP.opcode

    cpu.ac = ac
    assert cpu.sp == StackBase.for_mode(cpu.mode)

    cpu.step()  # copytosp

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Instruction.COPYTOSP
    assert cpu.sp == cpu.ac


@pytest.mark.parametrize("sp", list(range(1, 10)))
def test_cpu_microcode_copyfromsp(sp, cpu) -> None:
    cpu.memory.words[0] = Instruction.COPYFROMSP.opcode
    cpu.sp = sp

    assert cpu.ac == 0

    cpu.step()  # copyfromsp

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Instruction.COPYFROMSP.opcode
    assert cpu.ac == cpu.sp


@pytest.mark.parametrize("address", list(range(10, 100, 10)))
def test_cpu_microcode_jump(address, cpu) -> None:
    cpu.memory.words[0] = Instruction.JUMP.opcode
    cpu.memory.words[1] = address

    cpu.step()  # jump

    assert cpu.pc == address
    assert cpu.ir == Instruction.JUMP.opcode


@pytest.mark.parametrize("address", list(range(10, 100, 10)))
def test_cpu_microcode_jumpeq_true(address, cpu) -> None:
    cpu.memory.words[0] = Instruction.JUMPEQ.opcode
    cpu.memory.words[1] = address
    cpu.ac = 0

    cpu.step()  # jumpeq

    assert cpu.pc == address
    assert cpu.ir == Instruction.JUMPEQ.opcode


@pytest.mark.parametrize("address", list(range(10, 100, 10)))
def test_cpu_microcode_jumpeq_false(address, cpu) -> None:
    cpu.memory.words[0] = Instruction.JUMPEQ.opcode
    cpu.memory.words[1] = address

    cpu.ac = 1

    cpu.step()  # jumpeq

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Instruction.JUMPEQ.opcode


@pytest.mark.parametrize("address", list(range(10, 100, 10)))
def test_cpu_microcode_jumpne_true(address, cpu) -> None:
    cpu.memory.words[0] = Instruction.JUMPNE.opcode
    cpu.memory.words[1] = address

    cpu.ac = 1

    cpu.step()  # jumpne

    assert cpu.pc == address
    assert cpu.ir == Instruction.JUMPNE.opcode


@pytest.mark.parametrize("address", list(range(10, 100, 10)))
def test_cpu_microcode_jumpne_false(address, cpu) -> None:
    cpu.memory.words[0] = Instruction.JUMPNE.opcode
    cpu.memory.words[1] = address

    cpu.ac = 0

    cpu.step()  # jumpne

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Instruction.JUMPNE.opcode


@pytest.mark.parametrize("address", list(range(10, 100, 10)))
def test_cpu_microcode_call(address, cpu) -> None:
    cpu.memory.words[0] = Instruction.CALL.opcode
    cpu.memory.words[1] = address

    cpu.step()  # call

    assert cpu.pc == address
    assert cpu.ir == Instruction.CALL.opcode
    assert cpu.sp == StackBase.for_mode(cpu.mode) - 1
    assert cpu.operand == address
    assert cpu.pc == address
    assert cpu.memory.words[cpu.sp] == 2


@pytest.mark.parametrize("address", list(range(10, 100, 10)))
def test_cpu_microcode_ret(address, cpu) -> None:
    cpu.memory.words[0] = Instruction.CALL.opcode
    cpu.memory.words[1] = address
    cpu.memory.words[address] = Instruction.RET.opcode

    cpu.step()  # call

    assert cpu.pc == address

    cpu.step()  # ret

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Instruction.RET.opcode


@pytest.mark.parametrize("x", list(range(1, 10)))
def test_cpu_microcode_incx(x, cpu) -> None:
    cpu.memory.words[0] = Instruction.INCX.opcode
    cpu.x = x

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Instruction.INCX.opcode
    assert cpu.x == x + 1


@pytest.mark.parametrize("x", list(range(1, 10)))
def test_cpu_microcode_decx(x, cpu) -> None:
    cpu.memory.words[0] = Instruction.DECX.opcode

    cpu.x = x

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Instruction.DECX.opcode
    assert cpu.x == x - 1


def test_cpu_microcode_push_user_mode(cpu) -> None:
    cpu.memory.words[0] = Instruction.PUSH.opcode

    cpu.ac = 0xFF
    sp = cpu.sp

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Instruction.PUSH.opcode
    assert cpu.sp == sp - 1
    assert cpu.memory.words[cpu.sp] == 0xFF


def test_cpu_microcode_pop_user_mode_success(cpu) -> None:

    cpu.ac = 0xFF
    cpu.memory.words[0] = Instruction.PUSH.opcode
    cpu.memory.words[1] = Instruction.POP.opcode

    cpu.step()  # push

    cpu.ac = 0

    assert cpu.sp < StackBase.for_mode(cpu.mode)

    cpu.step()  # pop

    assert cpu.ir == Instruction.POP.opcode
    assert cpu.ac == 0xFF
    assert cpu.sp == StackBase.for_mode(cpu.mode)


def test_cpu_microcode_pop_user_mode_underflow(cpu) -> None:
    cpu.memory.words[0] = Instruction.POP.opcode

    with pytest.raises(StackUnderflowError):
        cpu.step()


def test_cpu_microcode_push_system_mode(cpu) -> None:

    cpu.memory.words[0] = Instruction.INTERRUPT.opcode
    cpu.memory.words[ProgramLoad.INTERRUPT.value] = Instruction.PUSH.opcode

    cpu.step()  # interrupt

    assert cpu.mode == Mode.SYSTEM
    assert cpu.pc == ProgramLoad.INTERRUPT.value
    assert cpu.sp == StackBase.for_mode(cpu.mode).value - 2

    cpu.ac = 0xFF

    cpu.step()  # push

    assert cpu.mode == Mode.SYSTEM
    assert cpu.pc == ProgramLoad.INTERRUPT.value + 1
    assert cpu.ir == Instruction.PUSH
    assert cpu.sp < StackBase.for_mode(cpu.mode)


def test_cpu_microcode_pop_system_mode_success(cpu) -> None:
    cpu.memory.words[0] = Instruction.INTERRUPT.opcode
    cpu.memory.words[ProgramLoad.INTERRUPT.value] = Instruction.PUSH.opcode
    cpu.memory.words[ProgramLoad.INTERRUPT.value + 1] = Instruction.POP.opcode

    cpu.step()  # interrupt

    assert cpu.mode == Mode.SYSTEM
    assert cpu.pc == ProgramLoad.INTERRUPT.value
    assert cpu.ir == Instruction.INTERRUPT

    cpu.ac = 0xFF

    cpu.step()  # push

    assert cpu.pc == ProgramLoad.INTERRUPT.value + 1
    assert cpu.ir == Instruction.PUSH
    assert cpu.sp < StackBase.for_mode(cpu.mode)

    cpu.ac = 0

    cpu.step()  # pop

    assert cpu.pc == ProgramLoad.INTERRUPT.value + 2
    assert cpu.ir == Instruction.POP
    assert cpu.ac == 0xFF


def test_cpu_microcode_pop_system_mode_underflow(cpu) -> None:
    cpu.memory.words[0] = Instruction.INTERRUPT.opcode
    cpu.memory.words[ProgramLoad.INTERRUPT.value] = Instruction.POP.opcode
    cpu.memory.words[ProgramLoad.INTERRUPT.value + 1] = Instruction.POP.opcode
    cpu.memory.words[ProgramLoad.INTERRUPT.value + 2] = Instruction.POP.opcode

    cpu.step()  # interrupt

    assert cpu.mode == Mode.SYSTEM
    assert cpu.pc == ProgramLoad.INTERRUPT.value
    assert cpu.ir == Instruction.INTERRUPT

    cpu.step()  # pop pc

    cpu.step()  # pop sp

    assert cpu.sp == StackBase.for_mode(cpu.mode)

    with pytest.raises(StackUnderflowError):
        cpu.step()  # pop


def test_cpu_microcode_interrupt_interrupts_enabled(cpu) -> None:
    cpu.memory.words[0] = Instruction.INTERRUPT.value

    assert cpu.mode == Mode.USER
    assert cpu.sp == StackBase.for_mode(cpu.mode)
    assert cpu.interrupts_enabled == True

    npc = cpu.pc + 1
    sp = cpu.sp

    cpu.step()

    assert cpu.mode == Mode.SYSTEM
    assert cpu.pc == ProgramLoad.INTERRUPT.value
    assert cpu.sp == StackBase.for_mode(cpu.mode) - 2
    assert cpu.interrupts_enabled == False
    assert cpu.memory.words[cpu.sp + 0] == npc
    assert cpu.memory.words[cpu.sp + 1] == sp


def test_cpu_microcode_interrupt_interrupts_disabled(cpu) -> None:
    cpu.memory.words[0] = Instruction.INTERRUPT.value

    assert cpu.mode == Mode.USER
    assert cpu.sp == StackBase.for_mode(cpu.mode)

    pc = cpu.pc
    sp = cpu.sp

    cpu.interrupts_enabled = False

    cpu.step()

    assert cpu.mode == Mode.USER
    assert cpu.pc == pc + 1
    assert cpu.sp == sp
    assert cpu.interrupts_enabled == False


def test_cpu_microcode_ireturn(cpu) -> None:
    cpu.memory.words[0] = Instruction.INTERRUPT.value
    cpu.memory.words[ProgramLoad.INTERRUPT.value] = Instruction.IRETURN.opcode

    assert cpu.interrupts_enabled == True

    sp = cpu.sp

    cpu.step()

    assert cpu.pc == ProgramLoad.INTERRUPT.value
    assert cpu.mode == Mode.SYSTEM
    assert cpu.interrupts_enabled == False

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Instruction.IRETURN.opcode
    assert cpu.sp == sp
    assert cpu.mode == Mode.USER
    assert cpu.interrupts_enabled == True


def test_cpu_microcode_end(cpu) -> None:

    cpu.memory.words[0] = Instruction.END.opcode

    with pytest.raises(StopIteration):
        cpu.step()
