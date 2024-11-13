pub const as = @import("./asm.zig");

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
    const Self = @This();

    root: [*]u64,
    alloc: std.mem.Allocator,
    offset: usize,

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

    pub fn blank(alloc: std.mem.Allocator, offset: usize) !Self {
        const root = try alloc.alloc(u8, std.mem.page_size);

        return .{
            .root = @alignCast(@ptrCast(root)),
            .alloc = alloc,
            .offset = offset,
        };
    }

    pub fn address(self: Self) u64 {
        return self.rmOffset(@intFromPtr(self.root));
    }

    pub fn addOffset(self: Self, offset: u64) usize {
        return offset + self.offset;
    }

    pub fn rmOffset(self: Self, offset: u64) usize {
        return offset - self.offset;
    }

    pub fn translateFlags(flags: u8) u64 {
        var f: u64 = Self.PageField.present | Self.PageField.noExecute;

        if (flags & MapFlag.none == MapFlag.none) {
            return 0;
        }

        if (flags & MapFlag.read == MapFlag.read) {}

        if (flags & MapFlag.write == MapFlag.write) {
            f |= Self.PageField.writable;
        }

        if (flags & MapFlag.execute == MapFlag.execute) {
            f &= ~Self.PageField.noExecute;
        }

        if (flags & MapFlag.user == MapFlag.user) {
            f |= Self.PageField.user;
        }

        if (flags & MapFlag.huge == MapFlag.huge) {
            f |= Self.PageField.huge;
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

    fn getEntry(self: *Self, index: usize, alloc: bool) !Self {
        if (self.root[index] & Self.PageField.present == Self.PageField.present) {
            return .{
                .root = @ptrFromInt(self.addOffset(Self.getEntryAddr(self.root[index]))),
                .alloc = self.alloc,
                .offset = self.offset,
            };
        }

        if (!alloc) {
            return error.PageNotFound;
        }

        const page = try Self.blank(self.alloc, self.offset);
        self.root[index] = self.rmOffset(@intFromPtr(page.root)) | Self.PageField.present | Self.PageField.writable | Self.PageField.user;
        return page;
    }

    pub fn mapPage(self: *Self, virt: u64, phys: u64, flags: u64) !void {
        std.debug.assert(virt % std.mem.page_size == 0);
        std.debug.assert(phys % std.mem.page_size == 0);

        const pml4Index = Self.getEntryIndex(virt, 3);
        const pml3Index = Self.getEntryIndex(virt, 2);
        const pml2Index = Self.getEntryIndex(virt, 1);
        const pml1Index = Self.getEntryIndex(virt, 0);

        var pml3 = try self.getEntry(pml4Index, true);

        if (flags & Self.PageField.huge == Self.PageField.huge and pSize == utils.gib(1)) {
            pml3.root[pml3Index] = phys | flags;
            return;
        }

        var pml2 = try pml3.getEntry(pml3Index, true);
        if (flags & Self.PageField.huge == Self.PageField.huge) {
            pml2.root[pml2Index] = phys | flags;
            return;
        }

        var pml1 = try pml2.getEntry(pml2Index, true);
        pml1.root[pml1Index] = phys | flags;
    }

    pub fn map(self: *Self, virt: u64, phys: u64, len: u64, flags: u8) !void {
        const _align: usize = if (flags & MapFlag.huge == MapFlag.huge) pSize else std.mem.page_size;

        const aligned_virt = std.mem.alignBackward(u64, virt, _align);
        const aligned_phys = std.mem.alignBackward(u64, phys, _align);
        const aligned_len = std.mem.alignForward(u64, len, _align);

        const f = Self.translateFlags(flags);

        var i: usize = 0;
        while (i < aligned_len) : (i += _align) {
            try self.mapPage(aligned_virt + i, aligned_phys + i, f);
        }
    }

    pub fn load(self: Self) void {
        as.cr3.write(@truncate(self.rmOffset(@intFromPtr(self.root))));
    }
};

fn mapSection(start: u64, end: u64, flags: u8) !void {
    if (limine.kaddr.response) |kaddr| {
        const aligned_start = std.mem.alignBackward(u64, start, std.mem.page_size);
        const len = std.mem.alignForward(u64, end - start, std.mem.page_size);

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

pub fn setup() !void {
    var allocator = pmm.PageAllocator.new();
    const alloc = allocator.allocator();

    kernelPage = try Space.blank(alloc, pmm.lower2upper(0));
    log.debug("Kernel page allocated at {x:0>16}", .{kernelPage.address()});

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
