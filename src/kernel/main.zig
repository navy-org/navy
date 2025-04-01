pub const loader = @import("loader");
pub const sched = @import("./sched.zig");
pub const syscall = @import("./syscall.zig");
pub const capability = @import("./capability.zig");

const std = @import("std");
const builtin = @import("builtin");
const arch = @import("arch");
const logger = @import("logger");
const elf = @import("elf");
const Task = @import("./task.zig").Task;
const log = std.log.scoped(.main);

pub const os = struct {
    pub const heap = struct {
        var page_alloc = arch.pmm.PageAllocator.new();
        pub const page_allocator = page_alloc.allocator();
    };
};

pub const std_options: std.Options = .{
    .log_level = if (builtin.mode == .Debug) .debug else .info,
    .logFn = logger.log,
    .page_size_max = arch.page_size_max,
    .page_size_min = arch.page_size_min,
};

pub var serial: arch.serial.Serial = undefined;

fn main() !void {
    serial = try arch.serial.Serial.init();
    try logger.setGlobalWriter(serial.writer());
    log.info("Hello, World!", .{});
    try arch.setup();
    try sched.setup();

    const servers: [2][]const u8 = .{ "/bin/bus", "/bin/init" };

    for (servers) |server| {
        if (arch.loader.findFile(server)) |srv| {
            const t = try Task.from_elf(server, elf.Elf.fromSlice(srv.address), &[_][]const u8{"MASSIVE"});
            try sched.push_task(t);
        } else {
            log.err("Couldn't load {s}", .{server});
            break;
        }
    }
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

export fn _start() callconv(.C) noreturn {
    main() catch |err| {
        log.err("Kernel fatal error: {}", .{err});
    };

    arch.as.hlt();
}
