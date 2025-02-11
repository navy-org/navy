const std = @import("std");
const navy = @import("navy");

pub const CAP_ID = 1;
pub const Services = enum(u8) { helo, ack };

pub const Ipc = struct {
    helo: Helo = .{},
    req_type: Services,

    pub fn recv_raw(capId: usize) !Ipc {
        var buffer: [64]u8 = undefined;
        _ = navy.read(capId, &buffer, 64);
        return from_buffer(buffer);
    }

    pub fn recv() !Ipc {
        return try recv_raw(CAP_ID);
    }

    pub fn from_buffer(buf: [64]u8) Ipc {
        const req: BusReq = @bitCast(buf[0..17].*);

        return switch (req.cmd) {
            .helo => .{ .helo = Helo.from_req(req), .req_type = .helo },
            .ack => .{ .req_type = .ack },
        };
    }
};

pub const BusReq = packed struct(u136) {
    cmd: Services,
    name: u128 = 0,

    pub fn send_raw(self: BusReq, capId: usize) u64 {
        var buffer: [17]u8 = undefined;
        std.mem.writeInt(u136, &buffer, @as(u136, @bitCast(self)), .little);
        return navy.write(capId, &buffer, buffer.len);
    }

    pub fn send(self: BusReq) u64 {
        return self.send_raw(CAP_ID);
    }
};

pub const Ack = struct {
    pub fn from_req(req: BusReq) Ack {
        _ = req;
        return Ack{};
    }

    pub fn send_raw(self: Ack, capId: usize) u64 {
        _ = self;
        const req: BusReq = .{
            .cmd = .ack,
        };

        return req.send_raw(capId);
    }

    pub fn send(self: Ack) u64 {
        return self.send_raw(CAP_ID);
    }
};

pub const Helo = struct {
    name: [16]u8 = std.mem.zeroes([16]u8),

    pub fn new(name: []const u8) Helo {
        var self: Helo = .{ .name = std.mem.zeroes([16]u8) };
        std.mem.copyForwards(u8, &self.name, name);
        return self;
    }

    pub fn from_req(req: BusReq) Helo {
        return Helo{ .name = @bitCast(req.name) };
    }

    pub fn send_raw(self: Helo, capId: usize) u64 {
        const req: BusReq = .{
            .cmd = .helo,
            .name = std.mem.readInt(u128, &self.name, .little),
        };

        return req.send_raw(capId);
    }

    pub fn send(self: Helo) u64 {
        return self.send_raw(CAP_ID);
    }
};
