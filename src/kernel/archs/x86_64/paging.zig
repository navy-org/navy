const as = @import("./asm.zig");
const cpuid = @import("./cpuid.zig");
const limine = @import("./limine.zig");
const logger = @import("logger");
const MapFlag = @import("hal").MapFlag;
const pmm = @import("./pmm.zig");
const std = @import("std");
const utils = @import("utils");

var kernelPage: Space = undefined;
var pSize: usize = utils.mib(2);
const log = std.log.scoped(.paging);

const PagingError = error{ AddressNotAligned, LengthNotAligned, PageNotFound, CantReadKernelAddr };

const Sections = struct {
    extern const text_start_addr: u64;
    extern const text_end_addr: u64;

    extern const rodata_start_addr: u64;
    extern const rodata_end_addr: u64;

    extern const data_start_addr: u64;
    extern const data_end_addr: u64;
};

pub const Space = struct {
    root: [*]u64,
    alloc: std.mem.Allocator,

    const PageField = struct {
        const present: u64 = 1 << 0;
        const writable: u64 = 1 << 1;
        const user: u64 = 1 << 2;
        const writeThrough: u64 = 1 << 3;
        const no_cache: u64 = 1 << 4;
        const accessed: u64 = 1 << 5;
        const dirty: u64 = 1 << 6;
        const huge: u64 = 1 << 7;
        const global: u64 = 1 << 8;
        const noExecute: u64 = 1 << 63;
    };

    pub fn blank() !Space {
        var pageAlloc = pmm.PageAllocator.new();
        var allocator = pageAlloc.allocator();
        const root = try allocator.alloc(u8, std.heap.pageSize());

        return .{
            .root = @alignCast(@ptrCast(root.ptr)),
            .alloc = allocator,
        };
    }

    pub fn create() !Space {
        const page = try Space.blank();

        for (255..512) |i| {
            page.root[i] = kernelPage.root[i];
        }

        return page;
    }

    pub fn translateFlags(flags: u8) u64 {
        var f: u64 = Space.PageField.present | Space.PageField.noExecute;

        if (flags & MapFlag.none == MapFlag.none) {
            return 0;
        }

        if (flags & MapFlag.read == MapFlag.read) {}

        if (flags & MapFlag.write == MapFlag.write) {
            f |= Space.PageField.writable;
        }

        if (flags & MapFlag.execute == MapFlag.execute) {
            f &= ~Space.PageField.noExecute;
        }

        if (flags & MapFlag.user == MapFlag.user) {
            f |= Space.PageField.user;
        }

        if (flags & MapFlag.huge == MapFlag.huge) {
            f |= Space.PageField.huge;
        }

        return f;
    }

    fn getEntryIndex(virt: u64, comptime level: u8) u64 {
        const shift: u64 = 12 + level * 9;
        return (virt & (0x1ff << shift)) >> shift;
    }

    fn getEntryAddr(addr: u64) u64 {
        return addr & 0x000ffffffffff000;
    }

    fn getEntry(self: Space, index: usize, alloc: bool) !Space {
        if (self.root[index] & Space.PageField.present == Space.PageField.present) {
            return .{
                .root = @ptrFromInt(pmm.lower2upper(Space.getEntryAddr(self.root[index]))),
                .alloc = self.alloc,
            };
        }

        if (!alloc) {
            return error.PageNotFound;
        }

        const page = try Space.blank();
        self.root[index] = pmm.upper2lower(@intFromPtr(page.root)) | Space.PageField.present | Space.PageField.writable | Space.PageField.user;
        return page;
    }

    pub fn mapPage(self: Space, virt: u64, phys: u64, flags: u64) !void {
        std.debug.assert(virt % std.heap.pageSize() == 0);
        std.debug.assert(phys % std.heap.pageSize() == 0);

        const pml4Index = Space.getEntryIndex(virt, 3);
        const pml3Index = Space.getEntryIndex(virt, 2);
        const pml2Index = Space.getEntryIndex(virt, 1);
        const pml1Index = Space.getEntryIndex(virt, 0);

        var pml3 = try self.getEntry(pml4Index, true);

        if (flags & Space.PageField.huge == Space.PageField.huge and pSize == utils.gib(1)) {
            pml3.root[pml3Index] = phys | flags;
            return;
        }

        var pml2 = try pml3.getEntry(pml3Index, true);
        if (flags & Space.PageField.huge == Space.PageField.huge) {
            pml2.root[pml2Index] = phys | flags;
            return;
        }

        var pml1 = try pml2.getEntry(pml2Index, true);
        pml1.root[pml1Index] = phys | flags;
    }

    fn unmapPage(self: Space, virt: u64) !void {
        const pml4Index = Space.getEntryIndex(virt, 3);
        const pml3Index = Space.getEntryIndex(virt, 2);
        const pml2Index = Space.getEntryIndex(virt, 1);
        const pml1Index = Space.getEntryIndex(virt, 0);

        const pml3 = try self.getEntry(pml4Index, false);
        const pml2 = try pml3.getEntry(pml3Index, false);
        const pml1 = try pml2.getEntry(pml2Index, false);

        const entry = &pml1.root[pml1Index];
        entry.* = 0;
    }

    pub fn virt2phys(self: Space, virt: u64) !u64 {
        const pml4Index = Space.getEntryIndex(virt, 3);
        const pml3Index = Space.getEntryIndex(virt, 2);
        const pml2Index = Space.getEntryIndex(virt, 1);
        const pml1Index = Space.getEntryIndex(virt, 0);

        const pml3 = try self.getEntry(pml4Index, false);
        const pml2 = try pml3.getEntry(pml3Index, false);
        const pml1 = try pml2.getEntry(pml2Index, false);

        const entry = pml1.root[pml1Index];
        return Space.getEntryAddr(entry);
    }

    pub fn map(self: Space, virt: u64, phys: u64, len: u64, flags: u8) !void {
        const _align: usize = if (flags & MapFlag.huge == MapFlag.huge) pSize else std.heap.pageSize();

        const aligned_virt = std.mem.alignBackward(u64, virt, _align);
        const aligned_phys = std.mem.alignBackward(u64, phys, _align);
        const aligned_len = std.mem.alignForward(u64, len, _align);

        const f = Space.translateFlags(flags);

        var i: usize = 0;
        while (i < aligned_len) : (i += _align) {
            try self.mapPage(aligned_virt + i, aligned_phys + i, f);
        }
    }

    pub fn unmap(self: Space, virt: u64, len: usize) !void {
        const aligned_virt = std.mem.alignBackward(u64, virt, std.heap.pageSize());
        const aligned_len = std.mem.alignBackward(u64, len, std.heap.pageSize());

        var i: usize = 0;
        while (i < aligned_len) : (i += std.heap.pageSize()) {
            try self.unmapPage(aligned_virt + i);
        }
    }

    pub fn load(self: Space) void {
        as.cr3.write(@truncate(pmm.upper2lower(@intFromPtr(self.root))));
    }
};

fn mapSection(start: u64, end: u64, flags: u8) !void {
    if (limine.kaddr.response) |kaddr| {
        const aligned_start = std.mem.alignBackward(u64, start, std.heap.pageSize());
        const len = std.mem.alignForward(u64, end - start, std.heap.pageSize());

        try kernelPage.map(
            aligned_start,
            (aligned_start - kaddr.virtual_base) + kaddr.physical_base,
            len,
            flags,
        );
    } else {
        return error.CantReadKernelAddr;
    }
}

pub fn kernelSpace() Space {
    return kernelPage;
}

pub fn setup() !void {
    kernelPage = try Space.blank();
    log.debug("Kernel page allocated at {x:0>16}", .{@intFromPtr(kernelPage.root)});

    if (try cpuid.has1GBPages()) {
        pSize = utils.gib(1);
        log.debug("1GB pages supported", .{});
    }

    try mapSection(
        @intFromPtr(&Sections.text_start_addr),
        @intFromPtr(&Sections.text_end_addr),
        MapFlag.read | MapFlag.execute,
    );

    try mapSection(
        @intFromPtr(&Sections.rodata_start_addr),
        @intFromPtr(&Sections.rodata_end_addr),
        MapFlag.read,
    );

    try mapSection(
        @intFromPtr(&Sections.data_start_addr),
        @intFromPtr(&Sections.data_end_addr),
        MapFlag.read | MapFlag.write,
    );

    log.debug("Kernel sections mapped", .{});

    try kernelPage.map(
        pmm.lower2upper(pSize),
        pSize,
        @max(utils.gib(4), pmm.availableMem()),
        MapFlag.write | MapFlag.read | MapFlag.huge,
    );

    if (limine.mmap.response) |mmap| {
        for (0..mmap.entry_count) |j| {
            const entry = mmap.entries()[j];
            if (entry.kind != limine.impl.MemoryMapEntryType.framebuffer) {
                try kernelPage.map(
                    pmm.lower2upper(entry.base),
                    entry.base,
                    entry.length,
                    MapFlag.read | MapFlag.write | MapFlag.huge,
                );
            }
        }
    } else {
        @panic("Mmmmh, how the pmm could use it, but now the paging can't? Anyway, serious error here");
    }

    log.debug("Kernel pages mapped", .{});
    kernelPage.load();
    log.debug("Paging loaded", .{});
}
