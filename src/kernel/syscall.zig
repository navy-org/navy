const std = @import("std");
const serial = @import("root").serial;
const sched = @import("./sched.zig");
const pmm = @import("arch").pmm;
const navy = @import("navy");
const MapFlags = @import("hal").MapFlag;

const Channel = @import("./channel.zig").Channel;
const AnyCap = @import("./capability.zig").AnyCap;
const CNode = @import("./capability.zig").CNode;
const Syscalls = @import("navy").Syscalls;

const log = std.log.scoped(.syscall);
const Error = error{ NoCapEntry, OutOfMemory, VmemFailed };

pub const SysArgs = struct {
    arg1: usize,
    arg2: usize,
    arg3: usize,
    arg4: usize,
    arg5: usize,
    arg6: usize,
};

fn write(capId: usize, bytes: [*]const u8, sz: usize) !u64 {
    const task = sched.current().?;

    const cap_idx = capId & 0xffff;

    if (cap_idx >= task.caps.items.len) {
        return error.NoCapEntry;
    }
    var cap: AnyCap = task.caps.items[cap_idx];

    while (cap.type == .cnode) {
        const entry = capId >> 16;
        if (entry == 0) {
            break;
        }

        const node: *CNode = @ptrCast(@alignCast(cap.context));

        if (entry - 1 >= node.caps.items.len) {
            return error.NoCapEntry;
        }

        cap = node.caps.items[entry - 1];
    }

    return try cap.write.?(cap.context, bytes[0..sz]);
}

fn read(capId: usize, buffer: [*]u8, sz: usize) !u64 {
    const task = sched.current().?;

    const cap_idx = capId & 0xffff;

    if (cap_idx >= task.caps.items.len) {
        return error.NoCapEntry;
    }

    var cap: AnyCap = task.caps.items[cap_idx];

    while (cap.type == .cnode) {
        const entry = capId >> 16;

        if (entry == 0) {
            break;
        }

        const node: *CNode = @ptrCast(@alignCast(cap.context));

        if (entry - 1 >= node.caps.items.len) {
            return error.NoCapEntry;
        }

        cap = node.caps.items[entry - 1];
    }

    return while (true) {
        const x = cap.read.?(cap.context, buffer[0..sz]) catch |err| {
            if (err == error.NoMessage) {
                continue;
            } else {
                return @intFromError(err);
            }
        };

        break x;
    };
}

fn mkchannel() !u64 {
    const task = sched.current().?;
    var channel = try Channel.new();
    const cap = channel.capability();
    try task.caps.append(cap);

    return task.caps.items.len;
}

fn mmap(addr: u64, len: usize, prot: u8) !u64 {
    const task = sched.current().?;

    _ = addr;
    // const aligned_addr = std.mem.alignBackward(u64, addr, std.heap.pageSize());
    const aligned_len = std.mem.alignForward(u64, len, std.heap.pageSize());

    const map_addr = task.mm.vmem.alloc(aligned_len, 0, 0);

    if (map_addr == null) {
        return Error.VmemFailed;
    }

    try task.mm.mmap_push_area(.{
        .start = @intFromPtr(map_addr),
        .end = @intFromPtr(map_addr) + aligned_len,
        .flags = prot | MapFlags.user,
    });

    return @intFromPtr(map_addr);
}

fn munmap(addr: u64, len: usize) !u64 {
    // const aligned_addr = std.mem.alignBackward(u64, addr, std.heap.pageSize());
    const aligned_len = std.mem.alignForward(u64, len, std.heap.pageSize());

    var task = sched.current().?;
    const slice = @as([*]u8, @ptrFromInt(pmm.lower2upper(try task.space.virt2phys(addr))));

    try task.space.unmap(addr, aligned_len);

    var palloc = pmm.PageAllocator.new();
    const alloc = palloc.allocator();
    alloc.free(slice[0..aligned_len]);

    task.mm.vmem.free(addr, aligned_len);
    task.mm.mmap_remove(addr);

    return 0;
}

pub fn handle(no: usize, args: SysArgs) u64 {
    const syscall: Syscalls = @enumFromInt(no);

    const ret = switch (syscall) {
        .write => write(args.arg1, @ptrFromInt(args.arg2), args.arg3),
        .read => read(args.arg1, @ptrFromInt(args.arg2), args.arg3),
        .mkchannel => mkchannel(),
        .mmap => mmap(args.arg1, args.arg2, @intCast(args.arg3)),
        .munmap => munmap(args.arg1, args.arg2),
    } catch |err| {
        const sys: Syscalls = @enumFromInt(no);
        std.log.err("{s} | Syscall {any} failed {}", .{ sched.current().?.name, sys, err });
        return @intFromError(err);
    };

    return ret;
}
