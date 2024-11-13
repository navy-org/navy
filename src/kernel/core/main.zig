const std = @import("std");
const builtin = @import("builtin");
const Serial = @import("arch").serial.Serial;
const logger = @import("logger");
const arch = @import("arch");
const log = std.log.scoped(.main);

pub const std_options = std.Options{
    .log_level = if (builtin.mode == .Debug) .debug else .info,
    .logFn = logger.log,
};

fn main() !void {
    var serial = try Serial.init();
    try logger.setGlobalWriter(serial.writer());
    log.info("Hello, World!", .{});
    try arch.setup();
}

pub fn panic(msg: []const u8, _: ?*std.builtin.StackTrace, ret_addr: ?usize) noreturn {
    logger.print("\nZig panic!\n", .{});
    logger.print("{s}\n\n", .{msg});

    if (ret_addr) |addr| {
        logger.print("Return address: {x}\n\n", .{addr});
    }

    logger.print("Stack trace:\n\n", .{});

    var iter = std.debug.StackIterator.init(ret_addr orelse @returnAddress(), null);
    defer iter.deinit();

    while (iter.next()) |address| {
        logger.print("    * 0x{x:0>16}\n", .{address});
    }

    arch.as.hlt();
}

export fn _start(magic: usize) callconv(.C) noreturn {
    if (magic != 0xB00B1E5) {
        log.err("Invalid magic number: 0x{x:0>16}", .{magic});
    }

    main() catch |err| {
        log.err("Kernel fatal error: {}", .{err});
    };

    arch.as.hlt();
}
