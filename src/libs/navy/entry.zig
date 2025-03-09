const std = @import("std");
const navy = @import("navy");
const builtin = @import("builtin");
const main = @import("main");

const logger = @import("logger");

pub const std_options = std.Options{
    .log_level = if (builtin.mode == .Debug) .debug else .info,
    .logFn = logger.log,
    // NOTE: Kinda of dirty but fine for now...
    .page_size_max = 4096,
};

pub const os = struct {
    pub const heap = struct {
        var page_alloc = navy.PageAllocator{};
        pub const page_allocator = page_alloc.allocator();
    };
};

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

    while (true) {}
    unreachable;
}

export fn _start() callconv(.C) noreturn {
    logger.setGlobalWriter(navy.Serial.writer()) catch {};
    main.main() catch |err| {
        std.log.err("error: {}", .{err});
    };
    while (true) {}
    unreachable;
}
