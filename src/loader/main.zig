const std = @import("std");
const builtin = @import("builtin");
const logger = @import("logger");
const kib = @import("utils").kib;
const arch = @import("arch");
const as = arch.as;

const mem = @import("./mem.zig");
const file = @import("./file.zig");

const EfiWriter = @import("./writer.zig").EfiWriter;
const EfiPageAllocator = mem.EfiPageAllocator;

const log = std.log.scoped(.loader);

pub const std_options = std.Options{
    .log_level = if (builtin.mode == .Debug) .debug else .info,
    .logFn = logger.log,
};

fn loaderMain() !void {
    try std.os.uefi.system_table.boot_services.?.setWatchdogTimer(0, 0, 0, null).err();

    const ei = try file.efiImage();
    var writer = EfiWriter.init();
    try logger.setGlobalWriter(writer.writer());

    log.info("Hello from the loader", .{});
    log.info("Base address: 0x{x}", .{@intFromPtr(ei.image_base)});

    try mem.setup();
    const entry = try file.loadElf("kernel.elf");
    log.debug("Kernel entry point: 0x{x:0>16}", .{entry});

    var allocator = EfiPageAllocator.new();
    const alloc = allocator.allocator();

    const stack: [*]u8 = @alignCast(@ptrCast(try alloc.alloc(u8, std.mem.page_size * 16)));
    const sp: usize = @intFromPtr(stack) + mem.higherHalf + std.mem.page_size * 16;

    log.debug("Entry: 0x{x:0>16}, stack: 0x{x:0>16}, space: 0x{x:0>16}", .{ entry, sp, mem.space.address() });

    const snapshot = try mem.EfiMemSnapshot.snapshot();
    try std.os.uefi.system_table.boot_services.?.exitBootServices(std.os.uefi.handle, snapshot.memory_map_key).err();

    // TODO: If using another architecture, this should be moved
    asm volatile (
        \\ cli
        \\ mov %[space], %cr3
        \\ call *%[entry]
        :
        : [sp] "{rsp}" (sp),
          [magic] "{rdi}" (0xB00B1E5),
          [_] "{rbp}" (0),
          [space] "r" (mem.space.address()),
          [entry] "r" (entry),
    );

    unreachable;
}

pub fn main() void {
    loaderMain() catch |err| {
        log.err("Loader fatal error: {any}", .{err});
    };

    while (true) {
        as.hlt();
    }
}

pub fn panic(msg: []const u8, _: ?*std.builtin.StackTrace, ret_addr: ?usize) noreturn {
    logger.print("\nZig panic!\n", .{});
    logger.print("{s}\n\n", .{msg});

    logger.print("Return address: {x:0>16}\n\n", .{ret_addr orelse @returnAddress()});

    logger.print("Stack trace:\n\n", .{});

    var iter = std.debug.StackIterator.init(ret_addr orelse @returnAddress(), null);
    defer iter.deinit();

    while (iter.next()) |address| {
        logger.print("    * 0x{x:0>16}\n", .{address});
    }

    while (true) {
        as.hlt();
    }
}
