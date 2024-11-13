const std = @import("std");
const arch = @import("arch");
const utils = @import("utils");
const file = @import("./file.zig");
const MapFlag = @import("hal").MapFlag;
const uefi = std.os.uefi;

const log = std.log.scoped(.loader);
pub const higherHalf: usize = 0xffff800000000000;
pub var space: arch.Space = undefined;

// --- Allocator ---------------------------------------------------------------

fn _alloc(_: *anyopaque, len: usize, _: u8, _: usize) ?[*]u8 {
    const npage = std.mem.alignForward(usize, len, std.mem.page_size) / std.mem.page_size;
    var memory: [*]align(std.mem.page_size) u8 = undefined;

    if (uefi.system_table.boot_services) |bs| {
        const status = bs.allocatePages(.AllocateAnyPages, .LoaderData, npage, &memory);
        if (status != uefi.Status.Success) {
            return null;
        }

        return memory;
    }

    return null;
}

fn _free(_: *anyopaque, buf: []u8, _: u8, _: usize) void {
    if (uefi.system_table.boot_services) |bs| {
        const status = bs.freePages(@alignCast(buf.ptr), buf.len / std.mem.page_size);
        if (status != uefi.Status.Success) {
            return;
        }
    }
}

pub const EfiPageAllocator = struct {
    pub fn new() EfiPageAllocator {
        return .{};
    }

    pub fn allocator(self: *EfiPageAllocator) std.mem.Allocator {
        return .{
            .ptr = self,
            .vtable = &.{ .alloc = _alloc, .free = _free, .resize = std.mem.Allocator.noResize },
        };
    }
};

// --- Memory Snapshot ---------------------------------------------------------

pub const EfiMemSnapshot = struct {
    memory_map: [*]uefi.tables.MemoryDescriptor,
    memory_map_size: usize,
    memory_map_key: usize,
    descriptor_size: usize,
    descriptor_version: u32,

    pub fn snapshot() !EfiMemSnapshot {
        var self: EfiMemSnapshot = .{ .memory_map = undefined, .memory_map_size = undefined, .memory_map_key = undefined, .descriptor_size = undefined, .descriptor_version = undefined };
        const bs = uefi.system_table.boot_services.?;

        while (bs.getMemoryMap(&self.memory_map_size, self.memory_map, &self.memory_map_key, &self.descriptor_size, &self.descriptor_version) == uefi.Status.BufferTooSmall) {
            try bs.allocatePool(uefi.tables.MemoryType.BootServicesData, self.memory_map_size, @ptrCast(&self.memory_map)).err();
        }

        return self;
    }
};

// --- Paging ------------------------------------------------------------------

pub fn setup() !void {
    var allocator = EfiPageAllocator.new();
    const alloc = allocator.allocator();

    space = try arch.Space.blank(alloc, 0);
    log.debug("Loader page allocated at 0x{x:0>16}", .{space.address()});

    const efiImage = try file.efiImage();
    log.debug("Mapping UEFI image at 0x{x:0>16} < 0x{x:0>16}", .{ @intFromPtr(efiImage.image_base), @intFromPtr(efiImage.image_base) + efiImage.image_size });

    try space.map(
        @intFromPtr(efiImage.image_base),
        @intFromPtr(efiImage.image_base),
        efiImage.image_size,
        MapFlag.read | MapFlag.write | MapFlag.execute,
    );

    log.debug("Mapping first 4 gib 0x{x:0>16} < 0x{x:0>16} to higher half", .{ std.mem.page_size, utils.gib(4) - std.mem.page_size });
    try space.map(
        std.mem.page_size + higherHalf,
        std.mem.page_size,
        utils.gib(4) - std.mem.page_size,
        MapFlag.write | MapFlag.read | MapFlag.huge,
    );
}
