const std = @import("std");
const PageAllocator = @import("arch").pmm.PageAllocator;
const Spinlock = @import("sync").Spinlock;
const kib = @import("utils").kib;
const sched = @import("./sched.zig");

const AnyCap = @import("./capability.zig").AnyCap;
const ChanError = error{NoMessage};

pub const Channel = struct {
    const CHANNEL_SIZE = kib(1);

    const Message = struct { sender: usize, msg: []u8 };
    const Messages = std.DoublyLinkedList(Message);

    var gpa = std.heap.GeneralPurposeAllocator(.{ .MutexType = Spinlock }){};
    const alloc = gpa.allocator();

    msg: Messages,

    refcount: usize,

    pub fn new() !*Channel {
        const self = try alloc.create(Channel);
        self.refcount = 2;
        self.msg = .{};
        return self;
    }

    pub fn capability(self: *Channel) AnyCap {
        return .{
            .type = .channel,
            .context = self,
            .read = readOpaque,
            .write = writeOpaque,
            .close = closeOpaque,
        };
    }

    fn writeOpaque(context: *anyopaque, bytes: []const u8) anyerror!usize {
        const ptr: *Channel = @ptrCast(@alignCast(context));
        return write(ptr, bytes);
    }

    fn readOpaque(context: *anyopaque, buffer: []u8) anyerror!usize {
        const ptr: *Channel = @ptrCast(@alignCast(context));
        return try read(ptr, buffer);
    }

    fn closeOpaque(context: *anyopaque) anyerror!void {
        const ptr: *Channel = @ptrCast(@alignCast(context));
        close(ptr);
    }

    fn write(self: *Channel, bytes: []const u8) !usize {
        const pid = sched.current().?.pid;

        var m = try alloc.create(Messages.Node);
        m.data = .{ .sender = pid, .msg = try alloc.alloc(u8, bytes.len) };
        @memcpy(m.data.msg, bytes);
        self.msg.append(m);
        return bytes.len;
    }

    fn read(self: *Channel, buffer: []u8) !usize {
        const pid = sched.current().?.pid;
        var message = self.msg.first;
        var msg: ?Message = null;
        var node: *Messages.Node = undefined;

        while (message) |m| : (message = m.next) {
            if (m.data.sender != pid) {
                msg = m.data;
                node = m;
                break;
            }
        }

        if (msg) |m| {
            @memset(buffer, 0);
            std.mem.copyForwards(u8, buffer, m.msg);
            self.msg.remove(node);
            return buffer.len;
        } else {
            return error.NoMessage;
        }
    }

    fn close(self: *Channel) void {
        self.refcount -= 1;

        if (self.refcount == 0) {
            var node = self.msg.first;
            while (node) |n| : (node = n.next) {
                alloc.destroy(node.?);
            }
        }
    }
};
