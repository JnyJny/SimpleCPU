/// CPU simulator constants

pub const Mode = enum {
    user,
    system,
};

pub const ProgramLoad = enum(usize) {
    user = 0,
    timer = 1000,
    interrupt = 1500,

    pub fn forMode(mode: Mode) ProgramLoad {
        return switch (mode) {
            .user => .user,
            .system => .interrupt,
        };
    }
};

pub const StackBase = enum(usize) {
    user = 999,
    system = 1999,

    pub fn forMode(mode: Mode) StackBase {
        return switch (mode) {
            .user => .user,
            .system => .system,
        };
    }

    pub fn value(self: StackBase) usize {
        return @intFromEnum(self);
    }
};

/// Total memory size: system stack top + 1
pub const NWORDS: usize = @intFromEnum(StackBase.system) + 1;

/// Magic number for object file format: b"%ejo"
pub const MAGIC: u32 = std.mem.readInt(u32, "%ejo", .big);

const std = @import("std");
