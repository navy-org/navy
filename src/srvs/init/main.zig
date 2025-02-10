const std = @import("std");
const log = std.log.scoped(.init);

const navy = @import("navy");

pub fn main() !void {
    var buffer: [64]u8 = undefined;

    _ = navy.write(navy.BUS, "HELLO", 5);
    log.info("Hello from init", .{});

    _ = navy.read(navy.BUS, &buffer, 64);
    log.info("Got message: {s}", .{buffer});
}
