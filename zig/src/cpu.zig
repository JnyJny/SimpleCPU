/// CPU simulator
const std = @import("std");
const constants = @import("constants.zig");
const CpuError = @import("error.zig").CpuError;
const Memory = @import("memory.zig").Memory;
const Opcode = @import("opcode.zig").Opcode;

pub const Cpu = struct {
    memory: Memory,
    timer_interval: usize,

    // Registers
    ir: ?Opcode,
    pc: usize,
    sp: usize,
    ac: i32,
    x: i32,
    y: i32,

    // State
    mode: constants.Mode,
    cycles: usize,
    interrupts_enabled: bool,

    // Output
    stdout: std.fs.File,

    pub fn init(memory: Memory, timer_interval: usize) Cpu {
        return .{
            .memory = memory,
            .timer_interval = timer_interval,
            .ir = null,
            .pc = @intFromEnum(constants.ProgramLoad.user),
            .sp = @intFromEnum(constants.StackBase.user),
            .ac = 0,
            .x = 0,
            .y = 0,
            .mode = .user,
            .cycles = 0,
            .interrupts_enabled = true,
            .stdout = std.fs.File{ .handle = std.posix.STDOUT_FILENO },
        };
    }

    /// Run the program to completion
    pub fn run(self: *Cpu) !void {
        while (true) {
            const continuing = try self.step();
            if (!continuing) return;
        }
    }

    /// Execute one instruction. Returns true to continue, false on END.
    pub fn step(self: *Cpu) !bool {
        if (self.fireTimer()) {
            try self.doInterrupt(.timer);
        }

        const raw = try self.load(self.pc);
        const opcode = try Opcode.fromInt(raw);
        self.ir = opcode;

        const operand: ?i32 = if (opcode.hasOperand())
            try self.load(self.pc + 1)
        else
            null;

        const continuing = try self.execute(opcode, operand);

        self.cycles += 1;

        if (continuing and !opcode.isCti()) {
            self.pc += if (operand != null) 2 else 1;
        }

        return continuing;
    }

    fn fireTimer(self: *const Cpu) bool {
        return self.cycles > 0 and
            self.timer_interval > 0 and
            self.cycles % self.timer_interval == 0;
    }

    fn load(self: *const Cpu, address: usize) CpuError!i32 {
        if (self.mode == .user and address > @intFromEnum(constants.StackBase.user)) {
            return CpuError.SegmentationFault;
        }
        return self.memory.read(address);
    }

    fn store(self: *Cpu, address: usize, value: i32) CpuError!void {
        if (self.mode == .user and address > @intFromEnum(constants.StackBase.user)) {
            return CpuError.SegmentationFault;
        }
        return self.memory.write(address, value);
    }

    fn push(self: *Cpu, value: i32) CpuError!void {
        self.sp -= 1;
        try self.store(self.sp, value);
    }

    fn pop(self: *Cpu) CpuError!i32 {
        const stackbase = @intFromEnum(constants.StackBase.forMode(self.mode));
        if (self.sp >= stackbase) {
            return CpuError.StackUnderflow;
        }
        const value = try self.load(self.sp);
        self.sp += 1;
        return value;
    }

    /// Write an integer to stdout
    fn putInt(self: *Cpu, value: i32) void {
        var buf: [32]u8 = undefined;
        const s = std.fmt.bufPrint(&buf, "{d}", .{value}) catch return;
        self.stdout.writeAll(s) catch {};
    }

    /// Write a single character to stdout
    fn putChar(self: *Cpu, value: i32) void {
        const byte = [1]u8{@intCast(@as(u32, @bitCast(value)) & 0xFF)};
        self.stdout.writeAll(&byte) catch {};
    }

    /// Execute a decoded instruction. Returns false on END.
    ///
    /// TODO: This is yours to fill in, Erik. The switch skeleton is here
    /// with a few examples to show the pattern. The rest are marked with
    /// @panic("TODO") so the compiler will tell you which ones are left.
    ///
    /// Hint: the Rust version in ../rust/src/cpu.rs has the complete logic
    /// if you get stuck on any particular opcode's semantics.
    fn execute(self: *Cpu, opcode: Opcode, operand: ?i32) !bool {
        switch (opcode) {
            // === LOADS (examples filled in) ===
            .loadv => {
                self.ac = operand.?;
            },
            .loada => {
                self.ac = try self.load(@intCast(operand.?));
            },
            .loadi => {
                const addr = try self.load(@intCast(operand.?));
                self.ac = try self.load(@intCast(addr));
            },
            .loadx => @panic("TODO: load value at address + X into AC"),
            .loady => @panic("TODO: load value at address + Y into AC"),
            .loadspx => @panic("TODO: load value at SP + X into AC"),

            // === STORE ===
            .store => @panic("TODO: store AC to address"),

            // === I/O ===
            .get => @panic("TODO: random int 1-100 into AC"),
            .put => {
                // port 1 = integer, port 2 = character
                const port = operand.?;
                switch (port) {
                    1 => self.putInt(self.ac),
                    2 => self.putChar(self.ac),
                    else => return CpuError.InvalidOperand,
                }
            },

            // === ARITHMETIC ===
            .addx => @panic("TODO: add X to AC"),
            .addy => @panic("TODO: add Y to AC"),
            .subx => @panic("TODO: subtract X from AC"),
            .suby => @panic("TODO: subtract Y from AC"),

            // === REGISTER COPIES ===
            .copytox => @panic("TODO: copy AC to X"),
            .copyfromx => @panic("TODO: copy X to AC"),
            .copytoy => @panic("TODO: copy AC to Y"),
            .copyfromy => @panic("TODO: copy Y to AC"),
            .copytosp => @panic("TODO: copy AC to SP"),
            .copyfromsp => @panic("TODO: copy SP to AC"),

            // === CONTROL FLOW ===
            .jump => {
                self.pc = @intCast(operand.?);
            },
            .jumpeq => @panic("TODO: jump if AC == 0, else pc += 2"),
            .jumpne => @panic("TODO: jump if AC != 0, else pc += 2"),
            .call => @panic("TODO: push return addr (pc+2), jump to operand"),
            .ret => @panic("TODO: pop return addr, jump to it"),

            // === STACK ===
            .incx => @panic("TODO: increment X"),
            .decx => @panic("TODO: decrement X"),
            .push => @panic("TODO: push AC onto stack"),
            .pop => @panic("TODO: pop stack into AC"),

            // === INTERRUPTS ===
            .interrupt => {
                try self.doInterrupt(.interrupt);
            },
            .ireturn => {
                try self.doIreturn();
            },

            // === END ===
            .end => return false,

            // Non-exhaustive enum catch-all
            _ => return CpuError.InvalidOpcode,
        }
        return true;
    }

    fn doInterrupt(self: *Cpu, target: constants.ProgramLoad) CpuError!void {
        if (!self.interrupts_enabled) {
            self.pc += 1;
            return;
        }

        self.mode = .system;
        self.interrupts_enabled = false;

        const u_sp = self.sp;
        var u_pc = self.pc;

        if (target == .interrupt) {
            u_pc += 1;
        }

        self.sp = @intFromEnum(constants.StackBase.system);
        try self.push(@intCast(u_sp));
        try self.push(@intCast(u_pc));

        self.pc = @intFromEnum(target);
    }

    fn doIreturn(self: *Cpu) CpuError!void {
        self.pc = @intCast(try self.pop());
        self.sp = @intCast(try self.pop());
        self.interrupts_enabled = true;
        self.mode = .user;
    }
};
