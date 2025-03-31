const std = @import("std");
const log = std.log.scoped(.vmem);
const Spinlock = @import("sync").Spinlock;

const C = @cImport({
    @cInclude("vmem.h");
});

pub const Config = struct {
    page_size: usize = std.heap.pageSize(),
    backing_allocator: std.mem.Allocator = std.heap.page_allocator,
};

pub const SLEEP = C.VM_SLEEP;
pub const NO_SLEEP = C.VM_NOSLEEP;

var lock: Spinlock = .init();
var bootstrap: bool = false;

export fn vmem_lock() callconv(.C) void {
    lock.lock();
}

export fn vmem_unlock() callconv(.C) void {
    lock.unlock();
}

pub fn Vmem(config: Config) type {
    return struct {
        const Self = @This();
        const backing_alloc: std.mem.Allocator = config.backing_allocator;
        const page_size: usize = config.page_size;

        vm: C.struct_vmem = .{},

        pub fn init(name: [:0]const u8, base: usize, size: usize, quantum: usize, flags: c_int) Self {
            if (!bootstrap) {
                C.vmem_bootstrap();
                bootstrap = true;
            }

            var self: Self = .{};
            _ = C.vmem_init(&self.vm, name, @ptrFromInt(base), size, quantum, null, null, null, 0, flags);
            return self;
        }

        export fn allocate_page(n: usize) callconv(.C) ?*anyopaque {
            const ptr = Self.backing_alloc.alloc(u8, n * Self.page_size) catch |e| {
                log.err("Unhandled error caught: {any}", .{e});
                return null;
            };

            return @ptrCast(ptr);
        }

        pub fn alloc(self: *Self, len: usize, min_addr: usize, max_addr: usize) ?[*]u8 {
            const min = if (min_addr > 0) min_addr else 0;
            const max = if (max_addr > 0) max_addr else std.math.maxInt(usize);

            const ptr: *anyopaque = C.vmem_xalloc(
                &self.vm,
                len,
                0,
                0,
                0,
                @ptrFromInt(min),
                @ptrFromInt(max),
                C.VM_BESTFIT,
                allocate_page,
            ) orelse return null;

            return @ptrCast(ptr);
        }

        pub fn free(self: *Self, addr: usize, len: usize) void {
            C.vmem_xfree(&self.vm, @ptrFromInt(addr), len);
        }
    };
}
