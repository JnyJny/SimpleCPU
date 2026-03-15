/// Simulator errors

pub const CpuError = error{
    InvalidOpcode,
    InvalidOperand,
    MachineCheck,
    MemoryRange,
    ObjectFormat,
    SegmentationFault,
    StackUnderflow,
};
