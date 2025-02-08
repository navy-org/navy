const std = @import("std");
const builtin = @import("builtin");

pub const Syscalls = enum(usize) {
    serial_out = 0,
};

pub const syscall = if (builtin.target.cpu.arch == .x86_64) @import("./x86_64.zig") else @compileError("This architecture is not supported");

pub const Serial = struct {
    pub fn write(bytes: []const u8) usize {
        return syscall.syscall2(.serial_out, @intFromPtr(bytes.ptr), bytes.len);
    }

    pub fn writeOpaque(context: *const anyopaque, bytes: []const u8) !usize {
        _ = context;
        return syscall.syscall2(.serial_out, @intFromPtr(bytes.ptr), bytes.len);
    }

    pub fn writer() std.io.AnyWriter {
        return .{ .context = @ptrFromInt(0xdeadbeef), .writeFn = writeOpaque };
    }
};
