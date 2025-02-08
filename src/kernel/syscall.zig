const Syscalls = @import("navy").Syscalls;
const serial = @import("root").serial;
const std = @import("std");
const log = std.log.scoped(.syscall);

pub const SysArgs = struct {
    arg1: usize,
    arg2: usize,
    arg3: usize,
    arg4: usize,
    arg5: usize,
    arg6: usize,
};

fn serial_out(s: [*]const u8, sz: usize) u64 {
    return serial.write(s[0..sz]);
}

pub fn handle(no: usize, args: SysArgs) u64 {
    const syscall: Syscalls = @enumFromInt(no);

    const ret = switch (syscall) {
        .serial_out => serial_out(@ptrFromInt(args.arg1), args.arg2),
    };

    return ret;
}
