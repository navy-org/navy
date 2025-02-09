const std = @import("std");
const navy = @import("navy");
const builtin = @import("builtin");
const main = @import("main");

const logger = @import("logger");

pub const std_options = std.Options{
    .log_level = if (builtin.mode == .Debug) .debug else .info,
    .logFn = logger.log,
};

export fn _start() callconv(.C) noreturn {
    logger.setGlobalWriter(navy.Serial.writer()) catch {};
    main.main() catch |err| {
        std.log.err("error: {}", .{err});
    };
    while (true) {}
    unreachable;
}
