const std = @import("std");
const log = std.log.scoped(.init);

const bus = @import("bus");

const Error = error{BUSDIDNTACK};

fn init() !void {
    const req = bus.Helo.new("init");
    _ = req.send();

    const resp = try bus.Ipc.recv();
    if (resp.req_type != .ack) {
        return error.BUSDIDNTACK;
    }
}

pub fn main() !void {
    try init();
    log.debug("Init is registered!", .{});
}
