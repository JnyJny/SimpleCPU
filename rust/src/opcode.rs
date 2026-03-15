/// Instruction opcodes for the SimpleCPU
use crate::error::CpuError;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct OpcodeInfo {
    pub opcode: i32,
    pub name: &'static str,
    pub is_cti: bool,
    pub has_operand: bool,
    pub description: &'static str,
}

/// All known opcodes
pub const OPCODES: &[OpcodeInfo] = &[
    OpcodeInfo { opcode: 1,  name: "loadv",      is_cti: false, has_operand: true,  description: "Load value into AC" },
    OpcodeInfo { opcode: 2,  name: "loada",      is_cti: false, has_operand: true,  description: "Load value from address into AC" },
    OpcodeInfo { opcode: 3,  name: "loadi",      is_cti: false, has_operand: true,  description: "Load value from address at address into AC" },
    OpcodeInfo { opcode: 4,  name: "loadx",      is_cti: false, has_operand: true,  description: "Load value at address + X into AC" },
    OpcodeInfo { opcode: 5,  name: "loady",      is_cti: false, has_operand: true,  description: "Load value at address + Y into AC" },
    OpcodeInfo { opcode: 6,  name: "loadspx",    is_cti: false, has_operand: false, description: "Load value at SP + X into AC" },
    OpcodeInfo { opcode: 7,  name: "store",      is_cti: false, has_operand: true,  description: "Store AC to address" },
    OpcodeInfo { opcode: 8,  name: "get",        is_cti: false, has_operand: false, description: "Get random int 1-100" },
    OpcodeInfo { opcode: 9,  name: "put",        is_cti: false, has_operand: true,  description: "Write AC to console" },
    OpcodeInfo { opcode: 10, name: "addx",       is_cti: false, has_operand: false, description: "Add X to AC" },
    OpcodeInfo { opcode: 11, name: "addy",       is_cti: false, has_operand: false, description: "Add Y to AC" },
    OpcodeInfo { opcode: 12, name: "subx",       is_cti: false, has_operand: false, description: "Subtract X from AC" },
    OpcodeInfo { opcode: 13, name: "suby",       is_cti: false, has_operand: false, description: "Subtract Y from AC" },
    OpcodeInfo { opcode: 14, name: "copytox",    is_cti: false, has_operand: false, description: "Copy AC to X" },
    OpcodeInfo { opcode: 15, name: "copyfromx",  is_cti: false, has_operand: false, description: "Copy X to AC" },
    OpcodeInfo { opcode: 16, name: "copytoy",    is_cti: false, has_operand: false, description: "Copy AC to Y" },
    OpcodeInfo { opcode: 17, name: "copyfromy",  is_cti: false, has_operand: false, description: "Copy Y to AC" },
    OpcodeInfo { opcode: 18, name: "copytosp",   is_cti: false, has_operand: false, description: "Copy AC to SP" },
    OpcodeInfo { opcode: 19, name: "copyfromsp", is_cti: false, has_operand: false, description: "Copy SP to AC" },
    OpcodeInfo { opcode: 20, name: "jump",       is_cti: true,  has_operand: true,  description: "Unconditional jump" },
    OpcodeInfo { opcode: 21, name: "jumpeq",     is_cti: true,  has_operand: true,  description: "Jump if AC is zero" },
    OpcodeInfo { opcode: 22, name: "jumpne",     is_cti: true,  has_operand: true,  description: "Jump if AC is not zero" },
    OpcodeInfo { opcode: 23, name: "call",       is_cti: true,  has_operand: true,  description: "Push return addr, jump to address" },
    OpcodeInfo { opcode: 24, name: "ret",        is_cti: true,  has_operand: false, description: "Pop return addr, jump" },
    OpcodeInfo { opcode: 25, name: "incx",       is_cti: false, has_operand: false, description: "Increment X" },
    OpcodeInfo { opcode: 26, name: "decx",       is_cti: false, has_operand: false, description: "Decrement X" },
    OpcodeInfo { opcode: 27, name: "push",       is_cti: false, has_operand: false, description: "Push AC onto stack" },
    OpcodeInfo { opcode: 28, name: "pop",        is_cti: false, has_operand: false, description: "Pop stack into AC" },
    OpcodeInfo { opcode: 29, name: "interrupt",  is_cti: true,  has_operand: false, description: "System call" },
    OpcodeInfo { opcode: 30, name: "ireturn",    is_cti: true,  has_operand: false, description: "Return from system call" },
    OpcodeInfo { opcode: 50, name: "end",        is_cti: false, has_operand: false, description: "End execution" },
];

/// Look up an opcode by its integer value
pub fn lookup(value: i32) -> crate::error::Result<&'static OpcodeInfo> {
    OPCODES
        .iter()
        .find(|op| op.opcode == value)
        .ok_or(CpuError::InvalidOpcode(value))
}
