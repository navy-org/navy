const lower2upper = @import("../pmm.zig").lower2upper;
const std = @import("std");
const log = std.log.scoped(.sdt);
const Either = @import("ds").Either;

const SdtError = error{
    InvalidChecksum,
    LookupFailedNotFound,
};

pub const SdtHeader = packed struct {
    const Self = @This();

    signature: u32,
    length: u32,
    revision: u8,
    checksum: u8,
    oem_id: u48,
    oem_table_id: u64,
    oem_revision: u32,
    creator_id: u32,
    creator_revision: u32,

    pub fn validate(self: *align(1) Self) !void {
        var sum: u8 = 0;
        const slice: [*]align(1) u8 = @ptrCast(self);

        for (0..self.length) |i| {
            sum +%= slice[i];
        }

        if (sum != 0) {
            return SdtError.InvalidChecksum;
        }
    }

    pub fn getSignature(self: *align(1) Self) []u8 {
        return @as([*]u8, @ptrCast(&self.signature))[0..4];
    }
};

pub fn SdtFactory(PointerLength: type) type {
    return packed struct {
        const Self = @This();

        header: SdtHeader,
        sdtAddr: void,

        pub fn len(self: Self) usize {
            return (self.header.length - @sizeOf(SdtHeader)) / @sizeOf(PointerLength);
        }

        pub fn lookup(self: *align(1) Self, t: type) !*align(1) t {
            const entries = @as([*]align(1) PointerLength, @ptrCast(@alignCast(&self.sdtAddr)))[0..self.len()];
            for (entries) |e| {
                const _sdt: *align(1) SdtHeader = @ptrFromInt(lower2upper(e));
                if (std.mem.eql(u8, _sdt.getSignature(), t.signature)) {
                    try _sdt.validate();
                    return @ptrCast(_sdt);
                }
            }

            return SdtError.LookupFailedNotFound;
        }

        pub fn fromAddress(address: usize) !*align(1) Self {
            const self: *align(1) Self = @ptrFromInt(address);
            try self.header.validate();
            return self;
        }
    };
}

pub const Rsdt = SdtFactory(u32);
pub const Xsdt = SdtFactory(u64);
pub const Sdt = Either(*align(1) Rsdt, *align(1) Xsdt);
pub var sdt: ?Sdt = null;
