const Syscalls = @import("navy").Syscalls;
const serial = @import("root").serial;
const std = @import("std");

const sched = @import("./sched.zig");
const Channel = @import("./channel.zig").Channel;
const AnyCap = @import("./capability.zig").AnyCap;

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
    var cap: AnyCap = task.caps.items[capId];

    return try cap.write.?(&cap, bytes[0..sz]);
}

fn mkchannel() !u64 {
    const task = sched.current();
    var channel = try Channel.create();
    const cap = channel.capability();
    try task.caps.append(cap);

    return task.caps.items.len;
}

pub fn handle(no: usize, args: SysArgs) u64 {
    const syscall: Syscalls = @enumFromInt(no);

    const ret = switch (syscall) {
        .write => write(args.arg1, @ptrFromInt(args.arg2), args.arg3),
        .mkchannel => mkchannel(),
    } catch {
        return 1;
    };

    return ret;
}
