/// Instruction opcodes for the SimpleCPU
use crate::error::CpuError;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(i32)]
pub enum Opcode {
    LoadV      = 1,
    LoadA      = 2,
    LoadI      = 3,
    LoadX      = 4,
    LoadY      = 5,
    LoadSpX    = 6,
    Store      = 7,
    Get        = 8,
    Put        = 9,
    AddX       = 10,
    AddY       = 11,
    SubX       = 12,
    SubY       = 13,
    CopyToX    = 14,
    CopyFromX  = 15,
    CopyToY    = 16,
    CopyFromY  = 17,
    CopyToSp   = 18,
    CopyFromSp = 19,
    Jump       = 20,
    JumpEq     = 21,
    JumpNe     = 22,
    Call       = 23,
    Ret        = 24,
    IncX       = 25,
    DecX       = 26,
    Push       = 27,
    Pop        = 28,
    Interrupt  = 29,
    IReturn    = 30,
    End        = 50,
}

impl TryFrom<i32> for Opcode {
    type Error = CpuError;

    fn try_from(value: i32) -> Result<Self, Self::Error> {
        match value {
            1  => Ok(Opcode::LoadV),
            2  => Ok(Opcode::LoadA),
            3  => Ok(Opcode::LoadI),
            4  => Ok(Opcode::LoadX),
            5  => Ok(Opcode::LoadY),
            6  => Ok(Opcode::LoadSpX),
            7  => Ok(Opcode::Store),
            8  => Ok(Opcode::Get),
            9  => Ok(Opcode::Put),
            10 => Ok(Opcode::AddX),
            11 => Ok(Opcode::AddY),
            12 => Ok(Opcode::SubX),
            13 => Ok(Opcode::SubY),
            14 => Ok(Opcode::CopyToX),
            15 => Ok(Opcode::CopyFromX),
            16 => Ok(Opcode::CopyToY),
            17 => Ok(Opcode::CopyFromY),
            18 => Ok(Opcode::CopyToSp),
            19 => Ok(Opcode::CopyFromSp),
            20 => Ok(Opcode::Jump),
            21 => Ok(Opcode::JumpEq),
            22 => Ok(Opcode::JumpNe),
            23 => Ok(Opcode::Call),
            24 => Ok(Opcode::Ret),
            25 => Ok(Opcode::IncX),
            26 => Ok(Opcode::DecX),
            27 => Ok(Opcode::Push),
            28 => Ok(Opcode::Pop),
            29 => Ok(Opcode::Interrupt),
            30 => Ok(Opcode::IReturn),
            50 => Ok(Opcode::End),
            _  => Err(CpuError::InvalidOpcode(value)),
        }
    }
}

impl Opcode {
    /// Whether this opcode is a control transfer instruction
    pub fn is_cti(self) -> bool {
        matches!(
            self,
            Opcode::Jump
                | Opcode::JumpEq
                | Opcode::JumpNe
                | Opcode::Call
                | Opcode::Ret
                | Opcode::Interrupt
                | Opcode::IReturn
        )
    }

    /// Whether this opcode consumes a second word as an operand
    pub fn has_operand(self) -> bool {
        matches!(
            self,
            Opcode::LoadV
                | Opcode::LoadA
                | Opcode::LoadI
                | Opcode::LoadX
                | Opcode::LoadY
                | Opcode::Store
                | Opcode::Put
                | Opcode::Jump
                | Opcode::JumpEq
                | Opcode::JumpNe
                | Opcode::Call
        )
    }

    pub fn description(self) -> &'static str {
        match self {
            Opcode::LoadV      => "Load value into AC",
            Opcode::LoadA      => "Load value from address into AC",
            Opcode::LoadI      => "Load value from address at address into AC",
            Opcode::LoadX      => "Load value at address + X into AC",
            Opcode::LoadY      => "Load value at address + Y into AC",
            Opcode::LoadSpX    => "Load value at SP + X into AC",
            Opcode::Store      => "Store AC to address",
            Opcode::Get        => "Get random int 1-100",
            Opcode::Put        => "Write AC to console",
            Opcode::AddX       => "Add X to AC",
            Opcode::AddY       => "Add Y to AC",
            Opcode::SubX       => "Subtract X from AC",
            Opcode::SubY       => "Subtract Y from AC",
            Opcode::CopyToX    => "Copy AC to X",
            Opcode::CopyFromX  => "Copy X to AC",
            Opcode::CopyToY    => "Copy AC to Y",
            Opcode::CopyFromY  => "Copy Y to AC",
            Opcode::CopyToSp   => "Copy AC to SP",
            Opcode::CopyFromSp => "Copy SP to AC",
            Opcode::Jump       => "Unconditional jump",
            Opcode::JumpEq     => "Jump if AC is zero",
            Opcode::JumpNe     => "Jump if AC is not zero",
            Opcode::Call       => "Push return addr, jump to address",
            Opcode::Ret        => "Pop return addr, jump",
            Opcode::IncX       => "Increment X",
            Opcode::DecX       => "Decrement X",
            Opcode::Push       => "Push AC onto stack",
            Opcode::Pop        => "Pop stack into AC",
            Opcode::Interrupt   => "System call",
            Opcode::IReturn    => "Return from system call",
            Opcode::End        => "End execution",
        }
    }
}
