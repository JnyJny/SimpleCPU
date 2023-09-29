"""
"""

import string

import pytest

from cpusim.cpu import CPU, SegmentationFault, StackUnderflowError
from cpusim.constants import Mode, ProgramLoad, StackBase
from cpusim.memory import Memory
from cpusim.opcode import Opcode, InvalidOpcodeError


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
    assert cpu.fault == False
    assert cpu.mode == Mode.USER

    assert cpu.ir == 0
    assert cpu.pc == ProgramLoad.USER.value
    assert cpu.sp == StackBase.for_mode(cpu.mode)
    assert cpu.ac == 0
    assert cpu.x == 0
    assert cpu.y == 0
    assert cpu.cycles == 0
    assert cpu.operand is None


def test_cpu_opcodes(cpu) -> None:

    for opcode in Opcode:
        assert getattr(cpu, opcode.name.lower())


def test_cpu_fire_timer(cpu) -> None:

    cpu.timer_interval = 100
    cpu.cycles = 100

    assert cpu.fire_timer == True

    cpu.cycles = 101

    assert cpu.fire_timer == False


@pytest.mark.parametrize("opcode", list(Opcode))
def test_cpu_opcode_attribute(opcode, cpu) -> None:

    cpu.ir = opcode.value
    assert cpu.opcode == opcode


def test_cpu_user_space_attribute(cpu) -> None:

    assert isinstance(cpu.user_space, range)


def test_cpu__load_method(cpu) -> None:
    for address in cpu.user_space:
        value = cpu._load(address)
        assert value == 0


def test_cpu__load_method_user_space_oob(cpu) -> None:

    for address_oob in cpu.user_space:
        with pytest.raises(SegmentationFault):
            cpu._load(address_oob + 1000)


def test_cpu__store_method(cpu) -> None:
    for address in cpu.user_space:
        cpu._store(address, 1)
        assert cpu.memory.words[address] == 1


def test_cpu__store_method_user_space_oob(cpu) -> None:

    for address_oob in cpu.user_space:
        with pytest.raises(SegmentationFault):
            cpu._store(address_oob + 1000, 1)
        assert cpu.memory.words[address_oob] != 1


def test_cpu__push_method_user_mode(cpu) -> None:

    base = StackBase.USER.value

    assert cpu.sp == base

    cpu._push(1)

    assert cpu.sp == base - 1
    assert cpu._load(cpu.sp + 1) == 1


def test_cpu__push_method_system_mode(cpu) -> None:

    cpu.mode = Mode.SYSTEM
    cpu.sp = StackBase.SYSTEM.value

    base = StackBase.SYSTEM.value

    assert cpu.sp == base

    cpu._push(1)

    assert cpu.sp == base - 1
    assert cpu._load(cpu.sp + 1) == 1


def test_cpu__pop_method_user_mode(cpu) -> None:

    base = StackBase.USER.value

    assert cpu.memory.words[base] != 1

    cpu.memory.words[base] = 1
    cpu.sp = base - 1

    value = cpu._pop()

    assert cpu.sp == base
    assert value == 1


def test_cpu__pop_method_system_mode(cpu) -> None:

    cpu.mode = Mode.SYSTEM
    base = StackBase.SYSTEM.value

    assert cpu.memory.words[base] != 1

    cpu.memory.words[base] = 1
    cpu.sp = base - 1

    value = cpu._pop()

    assert cpu.sp == base
    assert value == 1


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
    cpu.memory.words[0] = Opcode.END.value

    assert cpu.timer_interval == 0
    assert cpu.fire_timer == False

    cpu.start()

    assert cpu.mode == Mode.USER
    assert cpu.fault == False
    assert cpu.debug == False
    assert cpu.pc == 0
    assert cpu.ir == Opcode.END.value
    assert cpu.sp == StackBase.for_mode(cpu.mode)
    assert cpu.ac == 0
    assert cpu.x == 0
    assert cpu.y == 0
    assert cpu.cycles == 1
    assert cpu.operand == None


def test_cpu_microcode_invalid(cpu) -> None:

    cpu.memory.words[0] = Opcode.INVALID.value

    with pytest.raises(InvalidOpcodeError):
        cpu.step()


def test_cpu_microcode_loadv(cpu) -> None:
    cpu.memory.words[0] = Opcode.LOADV.value
    cpu.memory.words[1] = 1

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Opcode.LOADV.value
    assert cpu.ac == 1
    assert cpu.operand == 1


def test_cpu_microcode_loada(cpu) -> None:
    cpu.memory.words[0] = Opcode.LOADA.value
    cpu.memory.words[1] = 2
    cpu.memory.words[2] = 0xFF

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Opcode.LOADA.value
    assert cpu.ac == 0xFF
    assert cpu.operand == 2


def test_cpu_microcode_loadi(cpu) -> None:
    cpu.memory.words[0] = Opcode.LOADI.value
    cpu.memory.words[1] = 3
    cpu.memory.words[2] = 0xFF
    cpu.memory.words[3] = 2

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Opcode.LOADI.value
    assert cpu.ac == 0xFF
    assert cpu.operand == 3


def test_cpu_microcode_loadx(cpu) -> None:
    cpu.memory.words[0] = Opcode.LOADX.value
    cpu.memory.words[1] = 99
    cpu.memory.words[100] = 0xFF

    cpu.x = 1

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Opcode.LOADX.value
    assert cpu.ac == 0xFF
    assert cpu.operand == 99


def test_cpu_microcode_loady(cpu) -> None:
    cpu.memory.words[0] = Opcode.LOADY.value
    cpu.memory.words[1] = 99
    cpu.memory.words[100] = 0xFF

    cpu.y = 1

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Opcode.LOADY.value
    assert cpu.ac == 0xFF
    assert cpu.operand == 99


def test_cpu_microcode_loadspx(cpu) -> None:

    cpu.memory.words[0] = Opcode.LOADSPX.value
    cpu.memory.words[cpu.sp - 100] = 0xFF
    cpu.x = -100

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Opcode.LOADSPX.value
    assert cpu.ac == 0xFF


def test_cpu_microcode_store(cpu) -> None:
    cpu.memory.words[0] = Opcode.STORE.value
    cpu.memory.words[1] = 10
    cpu.ac = 0xFF

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Opcode.STORE.value
    assert cpu.operand == 10
    assert cpu.memory.words[10] == 0xFF


def test_cpu_microcode_get(cpu) -> None:
    cpu.memory.words[0] = Opcode.GET.value

    assert cpu.ac == 0

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Opcode.GET.value
    assert cpu.ac in range(1, 101)


@pytest.mark.parametrize("char", string.printable)
def test_cpu_microcode_put_port_1(char, cpu, capsys) -> None:

    cpu.memory.words[0] = Opcode.PUT.value
    cpu.memory.words[1] = 1

    cpu.ac = ord(char)

    cpu.step()

    captured = capsys.readouterr()

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Opcode.PUT.value
    assert cpu.operand == 1
    assert captured.out == str(cpu.ac)


@pytest.mark.parametrize("char", string.printable)
def test_cpu_microcode_put_port_2(char, cpu, capsys) -> None:
    cpu.memory.words[0] = Opcode.PUT.value
    cpu.memory.words[1] = 2

    cpu.ac = ord(char)

    cpu.step()

    captured = capsys.readouterr()

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Opcode.PUT.value
    assert cpu.operand == 2

    assert captured.out == chr(cpu.ac)


@pytest.mark.parametrize("ac, x", [(ac, x) for ac, x in zip(range(0, 10), (0, 10))])
def test_cpu_microcode_addx(ac, x, cpu) -> None:
    cpu.memory.words[0] = Opcode.ADDX.value
    cpu.ac = ac
    cpu.x = x

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Opcode.ADDX.value
    assert cpu.ac == ac + x


@pytest.mark.parametrize("ac, y", [(ac, y) for ac, y in zip(range(0, 10), (0, 10))])
def test_cpu_microcode_addy(ac, y, cpu) -> None:
    cpu.memory.words[0] = Opcode.ADDY.value
    cpu.ac = ac
    cpu.y = y

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Opcode.ADDY.value
    assert cpu.ac == ac + y


@pytest.mark.parametrize("ac, x", [(ac, x) for ac, x in zip(range(0, 10), (0, 10))])
def test_cpu_microcode_subx(ac, x, cpu) -> None:
    cpu.memory.words[0] = Opcode.SUBX.value

    cpu.ac = ac
    cpu.x = x

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Opcode.SUBX.value
    assert cpu.ac == ac - x


@pytest.mark.parametrize("ac, y", [(ac, y) for ac, y in zip(range(0, 10), (0, 10))])
def test_cpu_microcode_suby(ac, y, cpu) -> None:
    cpu.memory.words[0] = Opcode.SUBY.value

    cpu.ac = ac
    cpu.y = y

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Opcode.SUBY.value
    assert cpu.ac == ac - y


@pytest.mark.parametrize("ac", list(range(1, 10)))
def test_cpu_microcode_copytox(ac, cpu) -> None:
    cpu.memory.words[0] = Opcode.COPYTOX.value
    cpu.ac = ac

    assert cpu.x == 0

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Opcode.COPYTOX.value
    assert cpu.x == cpu.ac


@pytest.mark.parametrize("x", list(range(1, 10)))
def test_cpu_microcode_copyfromx(x, cpu) -> None:
    cpu.memory.words[0] = Opcode.COPYFROMX.value

    cpu.x = x
    assert cpu.ac == 0

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Opcode.COPYFROMX.value
    assert cpu.ac == cpu.x


@pytest.mark.parametrize("ac", list(range(1, 10)))
def test_cpu_microcode_copytoy(ac, cpu) -> None:
    cpu.memory.words[0] = Opcode.COPYTOY.value

    cpu.ac = ac

    assert cpu.y == 0

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Opcode.COPYTOY.value
    assert cpu.y == cpu.ac


@pytest.mark.parametrize("y", list(range(1, 10)))
def test_cpu_microcode_copyfromx(y, cpu) -> None:
    cpu.memory.words[0] = Opcode.COPYFROMY.value

    cpu.y = y

    assert cpu.ac == 0

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Opcode.COPYFROMY.value
    assert cpu.ac == cpu.y


@pytest.mark.parametrize("ac", list(range(1, 10)))
def test_cpu_microcode_copytosp(ac, cpu) -> None:
    cpu.memory.words[0] = Opcode.COPYTOSP.value

    cpu.ac = ac
    assert cpu.sp == StackBase.for_mode(cpu.mode)

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Opcode.COPYTOSP.value
    assert cpu.sp == cpu.ac


@pytest.mark.parametrize("sp", list(range(1, 10)))
def test_cpu_microcode_copyfromsp(sp, cpu) -> None:
    cpu.memory.words[0] = Opcode.COPYFROMSP.value
    cpu.sp = sp

    assert cpu.ac == 0

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Opcode.COPYFROMSP.value
    assert cpu.ac == cpu.sp


@pytest.mark.parametrize("address", list(range(10, 100, 10)))
def test_cpu_microcode_jump(address, cpu) -> None:
    cpu.memory.words[0] = Opcode.JUMP.value
    cpu.memory.words[1] = address

    cpu.step()

    assert cpu.pc == address
    assert cpu.ir == Opcode.JUMP.value


@pytest.mark.parametrize("address", list(range(10, 100, 10)))
def test_cpu_microcode_jumpeq_true(address, cpu) -> None:
    cpu.memory.words[0] = Opcode.JUMPEQ.value
    cpu.memory.words[1] = address
    cpu.ac = 0

    cpu.step()

    assert cpu.pc == address
    assert cpu.ir == Opcode.JUMPEQ.value


@pytest.mark.parametrize("address", list(range(10, 100, 10)))
def test_cpu_microcode_jumpeq_false(address, cpu) -> None:
    cpu.memory.words[0] = Opcode.JUMPEQ.value
    cpu.memory.words[1] = address

    cpu.ac = 1

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Opcode.JUMPEQ.value


@pytest.mark.parametrize("address", list(range(10, 100, 10)))
def test_cpu_microcode_jumpne_true(address, cpu) -> None:
    cpu.memory.words[0] = Opcode.JUMPNE.value
    cpu.memory.words[1] = address

    cpu.ac = 1

    cpu.step()

    assert cpu.pc == address
    assert cpu.ir == Opcode.JUMPNE.value


@pytest.mark.parametrize("address", list(range(10, 100, 10)))
def test_cpu_microcode_jumpne_false(address, cpu) -> None:
    cpu.memory.words[0] = Opcode.JUMPNE.value
    cpu.memory.words[1] = address

    cpu.ac = 0

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Opcode.JUMPNE.value


@pytest.mark.parametrize("address", list(range(10, 100, 10)))
def test_cpu_microcode_call(address, cpu) -> None:
    cpu.memory.words[0] = Opcode.CALL.value
    cpu.memory.words[1] = address

    cpu.step()

    assert cpu.pc == address
    assert cpu.ir == Opcode.CALL.value
    assert cpu.sp == StackBase.for_mode(cpu.mode) - 1
    assert cpu.operand == address
    assert cpu.pc == address
    assert cpu.memory.words[cpu.sp + 1] == 2


@pytest.mark.parametrize("address", list(range(10, 100, 10)))
def test_cpu_microcode_ret(address, cpu) -> None:
    cpu.memory.words[0] = Opcode.CALL.value
    cpu.memory.words[1] = address
    cpu.memory.words[address] = Opcode.RET.value

    cpu.step()

    assert cpu.pc == address

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 2
    assert cpu.ir == Opcode.RET.value


@pytest.mark.parametrize("x", list(range(1, 10)))
def test_cpu_microcode_incx(x, cpu) -> None:
    cpu.memory.words[0] = Opcode.INCX.value
    cpu.x = x

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Opcode.INCX.value
    assert cpu.x == x + 1


@pytest.mark.parametrize("x", list(range(1, 10)))
def test_cpu_microcode_decx(x, cpu) -> None:
    cpu.memory.words[0] = Opcode.DECX.value

    cpu.x = x

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Opcode.DECX.value
    assert cpu.x == x - 1


def test_cpu_microcode_push_user_mode(cpu) -> None:
    cpu.memory.words[0] = Opcode.PUSH.value

    cpu.ac = 0xFF
    sp = cpu.sp

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Opcode.PUSH.value
    assert cpu.sp == sp - 1
    assert cpu.memory.words[cpu.sp] == 0xFF


def test_cpu_microcode_pop_user_mode_success(cpu) -> None:

    cpu.ac = 0xFF
    cpu.memory.words[0] = Opcode.PUSH.value
    cpu.memory.words[1] = Opcode.POP.value

    cpu.step()

    cpu.ac = 0

    assert cpu.sp < StackBase.for_mode(cpu.mode)

    cpu.step()

    assert cpu.ir == Opcode.POP.value
    assert cpu.ac == 0xFF
    assert cpu.sp == StackBase.for_mode(cpu.mode)


def test_cpu_microcode_pop_user_mode_underflow(cpu) -> None:
    cpu.memory.words[0] = Opcode.POP.value

    with pytest.raises(StackUnderflowError):
        cpu.step()


def test_cpu_microcode_push_system_mode(cpu) -> None:
    cpu.memory.words[0] = Opcode.PUSH.value


def test_cpu_microcode_pop_system_mode_success(cpu) -> None:
    cpu.memory.words[0] = Opcode.POP.value


def test_cpu_microcode_pop_system_mode_underflow(cpu) -> None:
    cpu.memory.words[0] = Opcode.POP.value


def test_cpu_microcode_interrupt_interrupts_enabled(cpu) -> None:
    cpu.memory.words[0] = Opcode.INTERRUPT.value

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
    assert cpu.memory.words[cpu.sp + 1] == npc
    assert cpu.memory.words[cpu.sp + 2] == sp


def test_cpu_microcode_interrupt_interrupts_disabled(cpu) -> None:
    cpu.memory.words[0] = Opcode.INTERRUPT.value

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
    cpu.memory.words[0] = Opcode.INTERRUPT.value
    cpu.memory.words[ProgramLoad.INTERRUPT.value] = Opcode.IRETURN.value

    assert cpu.interrupts_enabled == True

    sp = cpu.sp

    cpu.step()

    assert cpu.pc == ProgramLoad.INTERRUPT.value
    assert cpu.mode == Mode.SYSTEM
    assert cpu.interrupts_enabled == False

    cpu.step()

    assert cpu.pc == ProgramLoad.USER.value + 1
    assert cpu.ir == Opcode.IRETURN.value
    assert cpu.sp == sp
    assert cpu.mode == Mode.USER
    assert cpu.interrupts_enabled == True


def test_cpu_microcode_end(cpu) -> None:

    cpu.memory.words[0] = Opcode.END.value

    with pytest.raises(StopIteration):
        cpu.step()
