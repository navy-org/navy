const std = @import("std");
const builtin = @import("builtin");
const logger = @import("logger");
const navy = @import("navy");

const log = std.log.scoped(.hello);

pub const std_options = std.Options{
    .log_level = if (builtin.mode == .Debug) .debug else .info,
    .logFn = logger.log,
};

pub fn main() !void {
    try logger.setGlobalWriter(navy.Serial.writer());
    log.debug("Hello from userspace !", .{});
}

export fn _start() callconv(.C) noreturn {
    main() catch |err| {
        log.err("Hello fatal error: {}", .{err});
    };

    while (true) {}
    unreachable;
}
