const std = @import("std");
const builtin = @import("builtin");

pub const Cap = usize;

pub const Syscalls = enum(usize) { write, read, mkchannel, mmap, munmap };

pub const MmapProt = struct {
    pub const None = 1 << 0;
    pub const Read = 1 << 1;
    pub const Write = 1 << 2;
    pub const Exec = 1 << 3;
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

pub const PageAllocator = struct {
    pub fn new() PageAllocator {
        return .{};
    }

    fn alloc(_: *anyopaque, len: usize, _: std.mem.Alignment, _: usize) ?[*]u8 {
        const slice = mmap(0, len, MmapProt.Read | MmapProt.Write);
        return slice.ptr;
    }

    fn free(_: *anyopaque, buf: []u8, _: std.mem.Alignment, _: usize) void {
        _ = munmap(@alignCast(buf));
    }

    pub fn allocator(self: *PageAllocator) std.mem.Allocator {
        return .{
            .ptr = self,
            .vtable = &.{
                .alloc = alloc,
                .free = free,
                .remap = std.mem.Allocator.noRemap,
                .resize = std.mem.Allocator.noResize,
            },
        };
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

pub fn mmap(addr: usize, len: usize, prot: usize) []align(std.heap.page_size_min) u8 {
    const ptr = syscall.syscall3(.mmap, addr, len, prot);
    return @as([*]align(std.heap.page_size_min) u8, @ptrFromInt(ptr))[0..len];
}

pub fn munmap(memory: []align(std.heap.page_size_min) u8) u64 {
    return syscall.syscall2(.munmap, @intFromPtr(memory.ptr), memory.len);
}
