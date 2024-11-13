const std = @import("std");

// TODO: Standard Zig allocator -> Selected nearest bucket
//       Non standard -> Create a bucket tailored to the size of the object

// TODO: If SMP implemented, this should be removed in favor of a dynamic CPU count.
const CPU_COUNT: usize = 1;

const SlabError = error{ObjectTypeTooSmall};

pub fn SlabAllocator(T: type) type {
    return struct {
        const Self = @This();

        // NOTE: This limit is arbitrary,
        //       I didn't want to implement a dynamic memory allocator
        //       for my dynamic memory allocator.
        const MAX_CORE_COUNT = 256;

        const Node = packed struct {
            const Inner = packed union {
                next: ?*Node,
                object: T,
            };
            used: bool,
            inner: Inner,
        };

        const Freelist = struct {
            head: ?*Node,
            tail: ?*Node,
            ptr: ?*Node,
        };

        const objectType = T;

        alloc: std.mem.Allocator,
        objectsPerSlab: usize,
        pageLength: usize,
        name: []const u8,
        cpuSlabs: [MAX_CORE_COUNT]Freelist,

        pub fn init(alloc: std.mem.Allocator) !Self {
            const sz = @bitSizeOf(T);
            var self = Self{
                .alloc = alloc,
                .name = @typeName(T),
                .objectsPerSlab = 0,
                .pageLength = 0,
                .cpuSlabs = undefined,
            };

            if (sz < 192) {
                self.pageLength = 1;
            } else if (sz < 512) {
                self.pageLength = 2;
            } else if (sz == 512) {
                self.pageLength = 4;
            } else {
                self.pageLength = 8;
            }

            self.objectsPerSlab = self.pageLength * std.mem.page_size / @sizeOf(Node);

            for (0..CPU_COUNT) |i| {
                var freelist: Freelist = .{ .head = null, .tail = null, .ptr = null };
                const slab: [*]u8 = @alignCast(@ptrCast(try self.alloc.alloc(u8, self.pageLength * std.mem.page_size)));

                for (0..self.objectsPerSlab) |j| {
                    const node: *Node = @ptrFromInt(@intFromPtr(slab) + j * @sizeOf(Node));
                    node.used = false;
                    if (freelist.head == null) {
                        freelist.head = node;
                        freelist.ptr = node;
                    } else {
                        freelist.ptr.?.inner.next = node;
                    }

                    freelist.tail = node;
                }

                self.cpuSlabs[i] = freelist;
            }

            return self;
        }

        // pub fn allocator(self: *Self) std.mem.Allocator {
        //     return .{
        //         .ptr = self,
        //         .vtable = &.{
        //         };
        //     };
        // }
    };
}

test "Simple Test" {
    const slab = SlabAllocator(u8);
    const alloc = std.heap.page_allocator;
    _ = try slab.init(alloc);
}
