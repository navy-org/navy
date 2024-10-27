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
    const Self = @This();

    root: [*]u64,
    alloc: std.mem.Allocator,

    const PageField = enum(u64) {
        Present = 1 << 0,
        Writable = 1 << 1,
        User = 1 << 2,
        WriteThrough = 1 << 3,
        No_cache = 1 << 4,
        Accessed = 1 << 5,
        Dirty = 1 << 6,
        Huge = 1 << 7,
        Global = 1 << 8,
        NoExecute = 1 << 63,
    };

    pub fn blank() !Self {
        var pageAlloc = pmm.PageAllocator.new();
        var allocator = pageAlloc.allocator();
        const root = try allocator.alloc(u8, std.mem.page_size);

        return .{
            .root = @alignCast(@ptrCast(root)),
            .alloc = allocator,
        };
    }

    pub fn translateFlags(flags: u8) u64 {
        var f: u64 = @intFromEnum(Self.PageField.Present) | @intFromEnum(Self.PageField.NoExecute);

        if (flags & @intFromEnum(MapFlag.None) == @intFromEnum(MapFlag.None)) {
            return 0;
        }

        if (flags & @intFromEnum(MapFlag.Read) == @intFromEnum(MapFlag.Read)) {}

        if (flags & @intFromEnum(MapFlag.Write) == @intFromEnum(MapFlag.Write)) {
            f |= @intFromEnum(Self.PageField.Writable);
        }

        if (flags & @intFromEnum(MapFlag.Execute) == @intFromEnum(MapFlag.Execute)) {
            f &= ~@intFromEnum(Self.PageField.NoExecute);
        }

        if (flags & @intFromEnum(MapFlag.User) == @intFromEnum(MapFlag.User)) {
            f |= @intFromEnum(Self.PageField.User);
        }

        if (flags & @intFromEnum(MapFlag.Huge) == @intFromEnum(MapFlag.Huge)) {
            f |= @intFromEnum(Self.PageField.Huge);
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
        if (self.root[index] & @intFromEnum(Self.PageField.Present) == @intFromEnum(Self.PageField.Present)) {
            return .{
                .root = @as([*]u64, @ptrFromInt(pmm.lower2upper(Self.getEntryAddr(self.root[index])))),
                .alloc = self.alloc,
            };
        }

        if (!alloc) {
            return error.PageNotFound;
        }

        const page = try Self.blank();
        self.root[index] = pmm.upper2lower(@intFromPtr(page.root)) | @intFromEnum(Self.PageField.Present) | @intFromEnum(Self.PageField.Writable) | @intFromEnum(Self.PageField.User);
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

        if (flags & @intFromEnum(Self.PageField.Huge) == @intFromEnum(Self.PageField.Huge) and pSize == utils.gib(1)) {
            pml3.root[pml3Index] = phys | flags;
            return;
        }

        var pml2 = try pml3.getEntry(pml3Index, true);
        if (flags & @intFromEnum(Self.PageField.Huge) == @intFromEnum(Self.PageField.Huge)) {
            pml2.root[pml2Index] = phys | flags;
            return;
        }

        var pml1 = try pml2.getEntry(pml2Index, true);
        pml1.root[pml1Index] = phys | flags;
    }

    pub fn map(self: *Self, virt: u64, phys: u64, len: u64, flags: u8) !void {
        const _align: usize = if (flags & @intFromEnum(MapFlag.Huge) == @intFromEnum(MapFlag.Huge)) pSize else std.mem.page_size;

        const aligned_virt = std.mem.alignBackward(u64, virt, _align);
        const aligned_phys = std.mem.alignBackward(u64, phys, _align);
        const aligned_len = std.mem.alignForward(u64, len, _align);

        const f = Self.translateFlags(flags);

        var i: usize = 0;
        while (i < aligned_len) : (i += _align) {
            try self.mapPage(aligned_virt, aligned_phys, f);
        }
    }

    pub fn load(self: Self) void {
        as.cr3.write(@truncate(pmm.upper2lower(@intFromPtr(self.root))));
    }
};

fn mapSection(start: u64, end: u64, flags: u8) !void {
    if (limine.kaddr.response) |kaddr| {
        const aligned_start = std.mem.alignBackward(u64, start, std.mem.page_size);
        const aligned_end = std.mem.alignForward(u64, end, std.mem.page_size);

        const f = Space.translateFlags(flags);
        var i: usize = aligned_start;

        while (i < aligned_end) : (i += std.mem.page_size) {
            const phys = i - kaddr.virtual_base + kaddr.physical_base;
            try kernelPage.mapPage(i, phys, f);
        }
    } else {
        return error.CantReadKernelAddr;
    }
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
        @intFromEnum(MapFlag.Read) | @intFromEnum(MapFlag.Execute),
    );

    try mapSection(
        @intFromPtr(&Sections.rodata_start_addr),
        @intFromPtr(&Sections.rodata_end_addr),
        @intFromEnum(MapFlag.Read),
    );

    try mapSection(
        @intFromPtr(&Sections.data_start_addr),
        @intFromPtr(&Sections.data_end_addr),
        @intFromEnum(MapFlag.Read) | @intFromEnum(MapFlag.Write),
    );

    log.debug("Kernel sections mapped", .{});

    try kernelPage.map(
        pmm.lower2upper(pSize),
        pSize,
        utils.gib(4),
        @intFromEnum(MapFlag.Write) | @intFromEnum(MapFlag.Read) | @intFromEnum(MapFlag.Huge),
    );

    if (limine.mmap.response) |mmap| {
        for (0..mmap.entry_count) |j| {
            const entry = mmap.entries()[j];
            if (entry.kind != limine.impl.MemoryMapEntryType.framebuffer) {
                try kernelPage.map(
                    pmm.lower2upper(entry.base),
                    entry.base,
                    entry.length,
                    @intFromEnum(MapFlag.Read) | @intFromEnum(MapFlag.Write) | @intFromEnum(MapFlag.Huge),
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
