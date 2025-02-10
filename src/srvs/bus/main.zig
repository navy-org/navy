const std = @import("std");
const log = std.log.scoped(.bus);
const read = @import("navy").read;
const write = @import("navy").write;

const BROADCAST = 1;
const INIT = 1 | 1 << 16;
const OTHER = 1 | 2 << 16;

pub fn main() !void {
    var buffer: [64]u8 = undefined;

    log.info("Hello from bus", .{});

    _ = read(INIT, &buffer, 64);
    log.info("Got message: {s}", .{buffer});

    _ = read(OTHER, &buffer, 64);
    log.info("Got message: {s}", .{buffer});

    _ = write(BROADCAST, "I'M THE BUS", 11);
}
