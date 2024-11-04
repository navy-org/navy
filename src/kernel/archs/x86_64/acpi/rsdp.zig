const std = @import("std");
const sdt = @import("./sdt.zig");
const lower2upper = @import("../pmm.zig").lower2upper;
const Either = @import("ds").Either;

const log = std.log.scoped(.rsdp);
var rsdp: ?*align(1) Rsdp = null;

const RsdpError = error{
    InvalidRsdpSignature,
    InvalidRsdpChecksum,
    XsdtUnavailable,
};

pub const Rsdp = extern struct {
    const Self = @This();
    const v1Length: u32 = 20;

    signature: [8]u8,
    checksum: u8,
    oem_id: [6]u8,
    revision: u8,
    rsdt_address: u32,

    // ACPI 2.0 or newer
    length: u32,
    xsdt_address: u64,
    extended_checksum: u8,
    reserved: [3]u8,

    fn validate(self: *align(1) Self) !void {
        if (!std.mem.eql(u8, &self.signature, "RSD PTR ")) {
            return RsdpError.InvalidRsdpSignature;
        }

        const length = if (self.revision > 0) self.length else Self.v1Length;

        var sum: u8 = 0;
        const slice: [*]align(1) u8 = @ptrCast(self);

        for (0..length) |i| {
            sum +%= slice[i];
        }

        if (sum != 0) {
            return RsdpError.InvalidRsdpChecksum;
        }
    }

    pub fn fromMem(address: *anyopaque) !*align(1) Self {
        log.debug("RSDP address: {x:0>16}", .{@intFromPtr(address)});
        const self: *align(1) Self = @ptrFromInt(lower2upper(@intFromPtr(address)));
        try self.validate();
        rsdp = self;
        return self;
    }

    pub fn findSdt(self: *align(1) Self) !sdt.Sdt {
        var value: sdt.Sdt = undefined;
        if (self.revision == 0) {
            log.debug("RSDT address: {x:0>16}", .{lower2upper(self.rsdt_address)});
            value = .{
                .left = try sdt.Rsdt.fromAddress(lower2upper(self.rsdt_address)),
                .right = null,
            };
        } else {
            log.debug("XSDT address: {x:0>16}", .{lower2upper(self.xsdt_address)});
            value = .{
                .right = try sdt.Xsdt.fromAddress(lower2upper(self.xsdt_address)),
                .left = null,
            };
        }

        sdt.sdt = value;
        return value;
    }
};
