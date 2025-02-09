const std = @import("std");
const AnyCap = @import("./capability.zig").AnyCap;
const PageAllocator = @import("arch").pmm.PageAllocator;
const kib = @import("utils").kib;

pub const Channel = struct {
    const CHANNEL_SIZE = kib(1);
    var palloc = PageAllocator.new();
    const alloc = palloc.allocator();

    buffer: []u8,
    seek: usize = 0,
    refcount: usize = 2,

    pub fn create() !Channel {
        return .{ .buffer = (try Channel.alloc.alloc(u8, CHANNEL_SIZE)) };
    }

    pub fn capability(self: *Channel) AnyCap {
        return .{
            .context = self,
            .read = readOpaque,
            .write = writeOpaque,
            .close = closeOpaque,
        };
    }

    fn writeOpaque(ptr: *anyopaque, buffer: []const u8) anyerror!usize {
        const self: *Channel = @ptrCast(@alignCast(ptr));
        return self.write(buffer);
    }

    fn readOpaque(ptr: *const anyopaque, bytes: []u8) anyerror!usize {
        const self: *const Channel = @ptrCast(@alignCast(ptr));
        return self.read(bytes);
    }

    fn closeOpaque(ptr: *anyopaque) anyerror!void {
        const self: *Channel = @ptrCast(@alignCast(ptr));
        self.close();
    }

    fn write(self: *Channel, bytes: []const u8) usize {
        @memset(self.buffer, 0);
        std.mem.copyForwards(u8, self.buffer, bytes);
        return bytes.len;
    }

    fn read(self: *const Channel, buffer: []u8) usize {
        @memset(buffer, 0);
        std.mem.copyForwards(u8, buffer, self.buffer);
        return self.buffer.len;
    }

    fn close(self: *Channel) void {
        self.refcount -= 1;

        if (self.refcount == 0) {
            alloc.free(self.buffer);
        }
    }
};
