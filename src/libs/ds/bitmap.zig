pub const Bitmap = struct {
    buf: [*]u8,
    len: usize,

    pub fn from_mem(mem: [*]u8, size: usize) Bitmap {
        return Bitmap{ .buf = mem, .len = size };
    }

    pub fn set(self: *Bitmap, bit: usize) void {
        self.buf[bit / 8] |= @as(u8, 1) << @truncate(bit % 8);
    }

    pub fn set_range(self: *Bitmap, start: usize, len: usize) void {
        for (start..(start + len)) |i| {
            self.set(i);
        }
    }

    pub fn is_set(self: *Bitmap, bit: usize) bool {
        return ((self.buf[bit / 8] >> @truncate(bit % 8)) & 1) == 1;
    }

    pub fn unset(self: *Bitmap, bit: usize) void {
        self.buf[bit / 8] &= ~(@as(u8, 1) << @truncate(bit % 8));
    }

    pub fn unset_range(self: *Bitmap, start: usize, len: usize) void {
        for (start..(start + len)) |i| {
            self.unset(i);
        }
    }

    pub fn fill(self: *Bitmap, value: u8) void {
        for (0..self.len) |i| {
            self.buf[i] = value;
        }
    }
};
