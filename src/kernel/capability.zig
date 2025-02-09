pub const AnyCap = struct {
    context: *anyopaque,

    read: ?*const fn (ptr: *const anyopaque, buffer: []u8) anyerror!usize,
    write: ?*const fn (ptr: *anyopaque, bytes: []const u8) anyerror!usize,
    close: ?*const fn (ptr: *anyopaque) anyerror!void,

    fn readOpaque(self: AnyCap, buffer: []u8) anyerror!void {
        return self.read.?(self.context, buffer);
    }

    fn writeOpaque(self: AnyCap, bytes: []const u8) anyerror!void {
        return self.write.?(self.context, bytes);
    }

    fn closeOpaque(self: AnyCap) anyerror!void {
        self.close.?(self.context);
    }
};
