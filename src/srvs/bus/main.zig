const std = @import("std");
const log = std.log.scoped(.bus);
const mkchannel = @import("navy").mkchannel;

pub fn main() !void {
    const channel = mkchannel();
    log.info("Hello from bus - {}", .{channel});
}
