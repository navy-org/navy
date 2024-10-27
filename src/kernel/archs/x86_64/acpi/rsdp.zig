const std = @import("std");
const logger = @import("logger");

const RsdpError = error{
    InvalidRsdpSignature,
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

    fn is_valid(self: *align(1) Self) bool {
        const ret = std.mem.eql(u8, &self.signature, "RSD PTR ");
        const length = if (self.revision > 0) self.length else Self.v1Length;

        var sum: u8 = 0;
        const slice: [*]align(1) u8 = @ptrCast(self);

        for (0..length) |i| {
            sum +%= slice[i];
        }

        const as_u8: u8 = @truncate(sum);
        return ret and as_u8 == 0;
    }

    pub fn from_address(address: *anyopaque) !*align(1) Self {
        const self: *align(1) Self = @alignCast(@ptrCast(address));
        if (!self.is_valid()) {
            return RsdpError.InvalidRsdpSignature;
        }

        return self;
    }
};
