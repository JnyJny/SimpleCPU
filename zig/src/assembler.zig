/// Assembler: parse source text into Memory
const std = @import("std");
const constants = @import("constants.zig");
const Memory = @import("memory.zig").Memory;

pub const Assembler = struct {
    memory: Memory,

    /// Parse assembly source from a file path
    pub fn fromFile(path: []const u8) !Assembler {
        const file = try std.fs.cwd().openFile(path, .{});
        defer file.close();

        const source = try file.readToEndAlloc(std.heap.page_allocator, 1024 * 1024);
        defer std.heap.page_allocator.free(source);

        return fromSource(source);
    }

    /// Parse assembly source text into memory
    pub fn fromSource(source: []const u8) !Assembler {
        var mem = Memory.init();
        var address: usize = @intFromEnum(constants.ProgramLoad.user);

        var lines = std.mem.splitScalar(u8, source, '\n');
        while (lines.next()) |line| {
            // Strip comments (anything after //)
            const text_end = std.mem.indexOf(u8, line, "//") orelse line.len;
            const text = std.mem.trim(u8, line[0..text_end], " \t\r");

            if (text.len == 0) continue;

            // Address directive: .1000
            if (text[0] == '.') {
                const addr_str = std.mem.trim(u8, text[1..], " \t");
                if (std.fmt.parseInt(usize, addr_str, 10)) |addr| {
                    address = addr;
                } else |_| {}
                continue;
            }

            // Parse first token as integer
            const token_end = std.mem.indexOfAny(u8, text, " \t") orelse text.len;
            if (std.fmt.parseInt(i32, text[0..token_end], 10)) |value| {
                try mem.write(address, value);
                address += 1;
            } else |_| {}
        }

        return .{ .memory = mem };
    }

    pub fn save(self: *const Assembler, path: []const u8) !void {
        try self.memory.save(path);
    }
};
