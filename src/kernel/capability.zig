const std = @import("std");
const Spinlock = @import("sync").Spinlock;
const Channel = @import("./channel.zig");

pub const AnyCap = struct {
    pub const Perm = struct {
        const read = 1 << 0;
        const write = 1 << 1;
        const exec = 1 << 2;
    };

    pub const Types = enum { cnode, channel, io };

    type: Types,
    context: *anyopaque,

    read: ?*const fn (ptr: *anyopaque, buffer: []u8) anyerror!usize,
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

    pub fn mint(self: AnyCap, perm: Perm) AnyCap {
        var minted_cap = self;

        if (perm & Perm.read == 0) {
            minted_cap.read = null;
        }

        if (perm & Perm.write == 0) {
            minted_cap.write = null;
        }

        if (perm & Perm.exec == 0) {
            // TODO:
        }

        return minted_cap;
    }
};

pub const CNode = struct {
    var gpa = std.heap.GeneralPurposeAllocator(.{ .MutexType = Spinlock }){};
    const alloc = gpa.allocator();

    caps: std.ArrayList(AnyCap),

    pub fn new() !*CNode {
        const self = try alloc.create(CNode);
        self.caps = std.ArrayList(AnyCap).init(alloc);
        return self;
    }

    pub fn capability(self: *CNode) AnyCap {
        return .{
            .type = .cnode,
            .context = self,
            .read = readOpaque,
            .write = writeOpaque,
            .close = closeOpaque,
        };
    }

    fn writeOpaque(context: *anyopaque, buffer: []const u8) anyerror!usize {
        const ptr: *CNode = @ptrCast(@alignCast(context));
        return write(ptr, buffer);
    }

    fn readOpaque(context: *anyopaque, buffer: []u8) anyerror!usize {
        const ptr: *CNode = @ptrCast(@alignCast(context));
        return read(ptr, buffer);
    }

    fn closeOpaque(context: *anyopaque) anyerror!void {
        const ptr: *CNode = @ptrCast(@alignCast(context));
        try close(ptr);
    }

    fn write(self: *CNode, bytes: []const u8) !usize {
        for (self.caps.items) |cap| {
            if (cap.write) |w| {
                _ = try w(cap.context, bytes);
            }
        }

        return bytes.len;
    }

    fn read(self: *CNode, buffer: []u8) !usize {
        var id: i64 = -1;
        for (0..self.caps.items.len, self.caps.items) |i, cap| {
            if (cap.read) |r| {
                const sz = r(cap.context, buffer) catch 0;
                if (sz != 0) {
                    id = @intCast(i);
                }
            }
        }

        return if (id != -1) @intCast(id + 1) else error.NoMessage;
    }

    fn close(self: *CNode) !void {
        for (self.caps.items) |cap| {
            try cap.close.?(cap.context);
        }
    }
};
