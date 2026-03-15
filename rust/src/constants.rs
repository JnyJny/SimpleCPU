/// CPU simulator constants

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Mode {
    User,
    System,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ProgramLoad {
    User = 0,
    Timer = 1000,
    Interrupt = 1500,
}

impl ProgramLoad {
    pub fn for_mode(mode: Mode) -> Self {
        match mode {
            Mode::User => ProgramLoad::User,
            Mode::System => ProgramLoad::Interrupt,
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum StackBase {
    User = 999,
    System = 1999,
}

impl StackBase {
    pub fn for_mode(mode: Mode) -> Self {
        match mode {
            Mode::User => StackBase::User,
            Mode::System => StackBase::System,
        }
    }
}

/// Total memory size: system stack top + 1
pub const NWORDS: usize = StackBase::System as usize + 1;

/// Magic number for object file format: b"%ejo"
pub const MAGIC: u32 = u32::from_be_bytes(*b"%ejo");
