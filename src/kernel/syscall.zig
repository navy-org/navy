const std = @import("std");
const serial = @import("root").serial;
const sched = @import("./sched.zig");

const Channel = @import("./channel.zig").Channel;
const AnyCap = @import("./capability.zig").AnyCap;
const CNode = @import("./capability.zig").CNode;
const Syscalls = @import("navy").Syscalls;

const log = std.log.scoped(.syscall);

pub const SysArgs = struct {
    arg1: usize,
    arg2: usize,
    arg3: usize,
    arg4: usize,
    arg5: usize,
    arg6: usize,
};

fn write(capId: usize, bytes: [*]const u8, sz: usize) !u64 {
    const task = sched.current();

    const cap_idx = capId & 0xffff;
    var cap: AnyCap = task.caps.items[cap_idx];

    while (cap.type == .cnode) {
        const entry = capId >> 16;
        if (entry == 0) {
            break;
        }

        const node: *CNode = @ptrCast(@alignCast(cap.context));
        cap = node.caps.items[entry - 1];
    }

    return try cap.write.?(cap.context, bytes[0..sz]);
}

fn read(capId: usize, buffer: [*]u8, sz: usize) !u64 {
    const task = sched.current();

    const cap_idx = capId & 0xffff;
    var cap: AnyCap = task.caps.items[cap_idx];

    while (cap.type == .cnode) {
        const entry = capId >> 16;

        if (entry == 0) {
            break;
        }

        const node: *CNode = @ptrCast(@alignCast(cap.context));
        cap = node.caps.items[entry - 1];
    }

    return try cap.read.?(cap.context, buffer[0..sz]);
}

fn mkchannel() !u64 {
    const task = sched.current();
    var channel = try Channel.new();
    const cap = channel.capability();
    try task.caps.append(cap);

    return task.caps.items.len;
}

pub fn handle(no: usize, args: SysArgs) u64 {
    const syscall: Syscalls = @enumFromInt(no);

    const ret = switch (syscall) {
        .write => write(args.arg1, @ptrFromInt(args.arg2), args.arg3),
        .read => read(args.arg1, @ptrFromInt(args.arg2), args.arg3),
        .mkchannel => mkchannel(),
    } catch {
        const sys: Syscalls = @enumFromInt(no);
        std.log.err("Syscall {any} failed", .{sys});
        return 1;
    };

    return ret;
}
