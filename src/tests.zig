const std = @import("std");

pub const slab = @import("./libs/alloc/slab.zig");

test {
    std.testing.refAllDecls(@This());
}
