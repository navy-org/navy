const std = @import("std");
const Vmem = @import("vmem").Vmem;
const gib = @import("mem").gib;
const Spinlock = @import("sync").Spinlock;

const USER_HEAP_BASE: usize = 0x80000000000;
const USER_HEAP_SIZE: usize = gib(4);

const MmapArea = struct { start: u64, end: u64, flags: u8 };
const MmapAreaList = std.DoublyLinkedList(MmapArea);

pub const TaskMM = struct {
    vmem: Vmem(.{}),
    mmap_areas: MmapAreaList = .{},

    var gpa = std.heap.GeneralPurposeAllocator(.{ .MutexType = Spinlock }){};
    const alloc = gpa.allocator();

    pub fn init(name: [:0]const u8) TaskMM {
        return .{
            .vmem = .init(name, USER_HEAP_BASE, USER_HEAP_SIZE, std.heap.pageSize(), 0),
        };
    }

    pub fn mmap_push_area(self: *TaskMM, area: MmapArea) !void {
        const len = std.mem.alignForward(usize, area.end - area.start, std.heap.pageSize());

        var i: usize = 0;

        while (i < len) : (i += std.heap.pageSize()) {
            const node = try alloc.create(MmapAreaList.Node);
            node.data = .{ .start = area.start + i, .end = area.start + i + std.heap.pageSize(), .flags = area.flags };
            self.mmap_areas.append(node);
        }
    }

    pub fn mmap_lookup(self: *TaskMM, addr: usize) ?MmapArea {
        var area = self.mmap_areas.first;
        while (area) |a| : (area = a.next) {
            if (addr >= a.data.start and addr < a.data.end) {
                return a.data;
            }
        }

        return null;
    }

    pub fn mmap_remove(self: *TaskMM, addr: usize) void {
        var area = self.mmap_areas.first;
        while (area) |a| : (area = a.next) {
            if (addr >= a.data.start and addr < a.data.end) {
                self.mmap_areas.remove(a);
            }
        }
    }
};
