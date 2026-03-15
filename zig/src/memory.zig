/// Memory subsystem for the SimpleCPU
const std = @import("std");
const constants = @import("constants.zig");
const CpuError = @import("error.zig").CpuError;

pub const Memory = struct {
    words: [constants.NWORDS]i32,

    pub fn init() Memory {
        return .{ .words = [_]i32{0} ** constants.NWORDS };
    }

    /// Load a binary object file (MAGIC header + i32 array)
    pub fn fromFile(path: []const u8) !Memory {
        const file = try std.fs.cwd().openFile(path, .{});
        defer file.close();

        const stat = try file.stat();
        if (stat.size < 4) return CpuError.ObjectFormat;

        var buf: [4]u8 = undefined;
        _ = try file.read(&buf);
        const magic = std.mem.readInt(u32, &buf, .big);
        if (magic != constants.MAGIC) return CpuError.ObjectFormat;

        var mem = init();
        const data_size = stat.size - 4;
        const nwords = data_size / 4;

        var i: usize = 0;
        while (i < nwords and i < constants.NWORDS) : (i += 1) {
            var word_buf: [4]u8 = undefined;
            _ = try file.read(&word_buf);
            mem.words[i] = @bitCast(word_buf);
        }

        return mem;
    }

    /// Save memory to a binary object file
    pub fn save(self: *const Memory, path: []const u8) !void {
        const file = try std.fs.cwd().createFile(path, .{});
        defer file.close();

        var magic_buf: [4]u8 = undefined;
        std.mem.writeInt(u32, &magic_buf, constants.MAGIC, .big);
        _ = try file.write(&magic_buf);

        for (self.words) |word| {
            const word_bytes: [4]u8 = @bitCast(word);
            _ = try file.write(&word_bytes);
        }
    }

    pub fn read(self: *const Memory, address: usize) CpuError!i32 {
        if (address >= constants.NWORDS) return CpuError.MemoryRange;
        return self.words[address];
    }

    pub fn write(self: *Memory, address: usize, value: i32) CpuError!void {
        if (address >= constants.NWORDS) return CpuError.MemoryRange;
        self.words[address] = value;
    }
};
