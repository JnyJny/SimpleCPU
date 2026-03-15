/// Instruction opcodes for the SimpleCPU
const CpuError = @import("error.zig").CpuError;

pub const Opcode = enum(i32) {
    loadv = 1,
    loada = 2,
    loadi = 3,
    loadx = 4,
    loady = 5,
    loadspx = 6,
    store = 7,
    get = 8,
    put = 9,
    addx = 10,
    addy = 11,
    subx = 12,
    suby = 13,
    copytox = 14,
    copyfromx = 15,
    copytoy = 16,
    copyfromy = 17,
    copytosp = 18,
    copyfromsp = 19,
    jump = 20,
    jumpeq = 21,
    jumpne = 22,
    call = 23,
    ret = 24,
    incx = 25,
    decx = 26,
    push = 27,
    pop = 28,
    interrupt = 29,
    ireturn = 30,
    end = 50,
    _,

    /// Decode an integer to an Opcode. Returns error on invalid values.
    pub fn fromInt(value: i32) CpuError!Opcode {
        return std.meta.intToEnum(Opcode, value) catch CpuError.InvalidOpcode;
    }

    /// Whether this opcode is a control transfer instruction
    pub fn isCti(self: Opcode) bool {
        return switch (self) {
            .jump, .jumpeq, .jumpne, .call, .ret, .interrupt, .ireturn => true,
            else => false,
        };
    }

    /// Whether this opcode consumes a second word as an operand
    pub fn hasOperand(self: Opcode) bool {
        return switch (self) {
            .loadv, .loada, .loadi, .loadx, .loady, .store, .put, .jump, .jumpeq, .jumpne, .call => true,
            else => false,
        };
    }

    /// Human-readable description
    pub fn description(self: Opcode) []const u8 {
        return switch (self) {
            .loadv => "Load value into AC",
            .loada => "Load value from address into AC",
            .loadi => "Load value from address at address into AC",
            .loadx => "Load value at address + X into AC",
            .loady => "Load value at address + Y into AC",
            .loadspx => "Load value at SP + X into AC",
            .store => "Store AC to address",
            .get => "Get random int 1-100",
            .put => "Write AC to console",
            .addx => "Add X to AC",
            .addy => "Add Y to AC",
            .subx => "Subtract X from AC",
            .suby => "Subtract Y from AC",
            .copytox => "Copy AC to X",
            .copyfromx => "Copy X to AC",
            .copytoy => "Copy AC to Y",
            .copyfromy => "Copy Y to AC",
            .copytosp => "Copy AC to SP",
            .copyfromsp => "Copy SP to AC",
            .jump => "Unconditional jump",
            .jumpeq => "Jump if AC is zero",
            .jumpne => "Jump if AC is not zero",
            .call => "Push return addr, jump to address",
            .ret => "Pop return addr, jump",
            .incx => "Increment X",
            .decx => "Decrement X",
            .push => "Push AC onto stack",
            .pop => "Pop stack into AC",
            .interrupt => "System call",
            .ireturn => "Return from system call",
            .end => "End execution",
            _ => "Unknown",
        };
    }
};

const std = @import("std");
