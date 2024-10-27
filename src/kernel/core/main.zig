const std = @import("std");
const builtin = @import("builtin");
const arch = @import("arch");
const logger = @import("logger");
const log = std.log.scoped(.main);

pub const std_options = std.Options{
    .log_level = if (builtin.mode == .Debug) .debug else .info,
    .logFn = logger.log,
};

fn main() !void {
    var serial = try arch.serial.Kwriter.init();
    try logger.setGlobalWriter(serial.writer());
    log.info("Hello, World!", .{});
    try arch.setup();
}

pub fn panic(msg: []const u8, stacktrace: ?*std.builtin.StackTrace, ret_addr: ?usize) noreturn {
    logger.print("\nZig panic!", .{});
    logger.print("{s}\n", .{msg});

    if (ret_addr) |addr| {
        logger.print("Return address: {x}\n", .{addr});
    }

    if (stacktrace) |trace| {
        logger.print("Stack trace:\n", .{});
        for (trace.instruction_addresses) |addr| {
            logger.print("  {}\n", .{addr});
        }
    }

    while (true) {
        asm volatile ("hlt");
    }
}

export fn _start() callconv(.C) noreturn {
    main() catch |err| {
        log.err("Kernel fatal error: {}", .{err});
    };

    while (true) {
        asm volatile ("hlt");
    }
}
