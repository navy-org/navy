const std = @import("std");
const builtin = @import("builtin");

pub const Cap = usize;

pub const Syscalls = enum(usize) {
    write,
    read,
    mkchannel,
};

pub const syscall = if (builtin.target.cpu.arch == .x86_64) @import("./x86_64.zig") else @compileError("This architecture is not supported");

pub const Serial = struct {
    pub fn writeOpaque(context: *const anyopaque, bytes: []const u8) !usize {
        _ = context;
        return write(0, bytes.ptr, bytes.len);
    }

    pub fn writer() std.io.AnyWriter {
        return .{ .context = @ptrFromInt(0xdeadbeef), .writeFn = writeOpaque };
    }
};

pub fn mkchannel() Cap {
    return syscall.syscall0(.mkchannel);
}

pub fn write(capId: usize, byte: [*]const u8, count: usize) u64 {
    return syscall.syscall3(.write, capId, @intFromPtr(byte), count);
}

pub fn read(capId: usize, buffer: [*]u8, count: usize) u64 {
    return syscall.syscall3(.read, capId, @intFromPtr(buffer), count);
}
