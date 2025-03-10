pub const vmem = @import("./vmem.zig");

pub fn kib(comptime x: usize) usize {
    return x * 1024;
}

pub fn mib(comptime x: usize) usize {
    return kib(x) * 1024;
}

pub fn gib(comptime x: usize) usize {
    return mib(x) * 1024;
}
