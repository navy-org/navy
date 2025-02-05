const std = @import("std");

pub const Spinlock = struct {
    locked: u32 = 0,

    pub fn init() Spinlock {
        return Spinlock{};
    }

    pub fn lock(self: *Spinlock) void {
        while (@cmpxchgWeak(
            u32,
            &self.locked,
            0,
            1,
            std.builtin.AtomicOrder.seq_cst,
            std.builtin.AtomicOrder.seq_cst,
        ) != null) {}
    }

    pub fn unlock(self: *Spinlock) void {
        if (@cmpxchgStrong(
            u32,
            &self.locked,
            1,
            0,
            std.builtin.AtomicOrder.seq_cst,
            std.builtin.AtomicOrder.seq_cst,
        ) != null) {
            @panic("releasing unheld SpinLock");
        }
    }
};
