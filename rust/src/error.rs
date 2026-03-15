/// Simulator errors
use thiserror::Error;

#[derive(Error, Debug)]
pub enum CpuError {
    #[error("InvalidOpcode: {0}")]
    InvalidOpcode(i32),

    #[error("InvalidOperand: {0}")]
    InvalidOperand(String),

    #[error("MachineCheck: {0}")]
    MachineCheck(String),

    #[error("MemoryRange: address {0} not in bounds")]
    MemoryRange(usize),

    #[error("ObjectFormat: {0}")]
    ObjectFormat(String),

    #[error("SegmentationFault: {0}")]
    SegmentationFault(String),

    #[error("StackUnderflow: sp={0} in mode {1:?}")]
    StackUnderflow(usize, Mode),

    #[error("IO: {0}")]
    Io(#[from] std::io::Error),
}

pub type Result<T> = std::result::Result<T, CpuError>;

use crate::constants::Mode;
