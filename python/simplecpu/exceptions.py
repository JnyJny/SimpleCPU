"""Simulator Exceptions
"""


class BaseException(Exception):
    def __init__(self, *args, **kwds) -> None:
        super().__init__(*args, **kwds)
        self.name = self.__class__.__name__

    def __str__(self) -> str:
        return f"{self.name}: {' '.join(self.args)}"


class InvalidOpcodeError(BaseException):
    """The requested opcode is invalid."""

    pass


class InvalidOperandError(BaseException):
    """The instruction operand is invalid."""

    pass


class MachineCheck(BaseException):
    """A machine error has occurred."""

    pass


class MemoryRangeError(BaseException):
    """The requested address is out of range."""

    pass


class ObjectFormatError(BaseException):
    """Malformed object file format."""

    pass


class SegmentationFault(BaseException):
    """Access to address requires elevated permissions."""

    pass


class StackUnderflowError(BaseException):
    """Popping the bottom of the stack."""

    pass


__all__ = [
    "InvalidOpcodeError",
    "InvalidOperandError",
    "MachineCheck",
    "MemoryRangeError",
    "ObjectFormatError",
    "SegmentationFault",
    "StackUnderflowError",
]
