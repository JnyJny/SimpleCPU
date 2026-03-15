const std = @import("std");
const Assembler = @import("assembler.zig").Assembler;
const Cpu = @import("cpu.zig").Cpu;
const Memory = @import("memory.zig").Memory;
const CpuError = @import("error.zig").CpuError;

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();

    const args = try std.process.argsAlloc(gpa.allocator());
    defer std.process.argsFree(gpa.allocator(), args);

    if (args.len < 2) {
        printUsage();
        std.process.exit(1);
    }

    const command = args[1];

    if (std.mem.eql(u8, command, "run")) {
        cmdRun(args[2..]) catch |err| {
            std.debug.print("Error: {}\n", .{err});
            std.process.exit(1);
        };
    } else if (std.mem.eql(u8, command, "asm")) {
        cmdAsm(args[2..]) catch |err| {
            std.debug.print("Error: {}\n", .{err});
            std.process.exit(1);
        };
    } else {
        std.debug.print("Unknown command: {s}\n", .{command});
        std.process.exit(1);
    }
}

fn printUsage() void {
    std.debug.print(
        \\Usage: simplecpu <command> [args]
        \\Commands:
        \\  run <file> [-t interval]  Run a program
        \\  asm <source> [-o output]  Assemble source
        \\
    , .{});
}

fn cmdRun(args: []const []const u8) !void {
    if (args.len < 1) {
        std.debug.print("Usage: simplecpu run <file> [-t interval]\n", .{});
        std.process.exit(1);
    }

    const path = args[0];
    var timer_interval: usize = 100;

    // Parse -t flag
    var i: usize = 1;
    while (i < args.len) : (i += 1) {
        if (std.mem.eql(u8, args[i], "-t") and i + 1 < args.len) {
            timer_interval = std.fmt.parseInt(usize, args[i + 1], 10) catch 100;
            i += 1;
        }
    }

    // Try object file first, fall back to assembly
    const memory = Memory.fromFile(path) catch |err| blk: {
        if (err == CpuError.ObjectFormat or err == error.FileNotFound) {
            const asm_result = Assembler.fromFile(path) catch |asm_err| {
                std.debug.print("Error loading {s}: {}\n", .{ path, asm_err });
                std.process.exit(1);
            };
            break :blk asm_result.memory;
        }
        std.debug.print("Error: {}\n", .{err});
        std.process.exit(1);
    };

    var cpu = Cpu.init(memory, timer_interval);
    try cpu.run();
}

fn cmdAsm(args: []const []const u8) !void {
    if (args.len < 1) {
        std.debug.print("Usage: simplecpu asm <source> [-o output]\n", .{});
        std.process.exit(1);
    }

    const source = args[0];
    var output: []const u8 = "out.o";

    // Parse -o flag
    var i: usize = 1;
    while (i < args.len) : (i += 1) {
        if (std.mem.eql(u8, args[i], "-o") and i + 1 < args.len) {
            output = args[i + 1];
            i += 1;
        }
    }

    const asm_result = try Assembler.fromFile(source);
    try asm_result.save(output);
}
