const std = @import("std");
const log = std.log.scoped(.gdt);
const PageAllocator = @import("./pmm.zig").PageAllocator;
const kib = @import("utils").kib;

extern fn gdt_flush(addr: u64) void;
extern fn tss_flush() void;

pub const GdtType = enum(u64) {
    Null = 0,
    KernelCode = 1,
    KernelData = 2,
    UserData = 3,
    UserCode = 4,

    Len = 5,
};

const GdtAccess = enum(u8) {
    ReadWrite = (1 << 1),
    Executable = (1 << 3),
    Descriptor = (1 << 4),
    User = (3 << 5),
    Present = (1 << 7),
};

const GdtFlags = enum(u8) {
    LongMode = (1 << 1),
    Size = (1 << 2),
    Granularity = (1 << 3),
};

const TssFlags = enum(u8) {
    Present = (1 << 7),
    LongModeAvailable = (0x9),
};

const GdtEntry = packed struct {
    limit_low: u16,
    base_low: u16,
    base_middle: u8,
    access: u8,
    limit_high: u4,
    flags: u4,
    base_high: u8,

    pub fn fast_init(access: u8, flags: u8) GdtEntry {
        var entry = std.mem.zeroes(GdtEntry);

        entry.access = access | @intFromEnum(GdtAccess.Present) | @intFromEnum(GdtAccess.ReadWrite) | @intFromEnum(GdtAccess.Descriptor);
        entry.flags = @intCast(flags | @intFromEnum(GdtFlags.Granularity));

        entry.limit_low = 0xffff;
        entry.limit_high = 0x0f;

        return entry;
    }
};

const Tss = packed struct {
    reserved0: u32,
    rsp: packed struct {
        rsp0: u64,
        rsp1: u64,
        rsp2: u64,
    },
    reserved1: u64,
    ist: packed struct {
        ist0: u64,
        ist1: u64,
        ist2: u64,
        ist3: u64,
        ist4: u64,
        ist5: u64,
        ist6: u64,
    },
    reserved2: u80,
    iopb_offset: u16,
};

const TssEntry = packed struct {
    length: u16,
    base_low: u16,
    base_middle: u8,
    flags: u16,
    base_high: u8,
    base_upper: u32,
    reserved: u32 = 0,

    pub fn from_addr(addr: u64) TssEntry {
        return .{
            .length = @intCast(@sizeOf(Tss)),
            .base_low = @intCast(addr & 0xffff),
            .base_middle = @intCast((addr >> 16) & 0xff),
            .base_high = @intCast((addr >> 24) & 0xff),
            .base_upper = @intCast(addr >> 32),
            .flags = @intCast(@intFromEnum(TssFlags.Present) | @intFromEnum(TssFlags.LongModeAvailable)),
        };
    }
};

const Gdt = extern struct {
    entries: [@intFromEnum(GdtType.Len)]GdtEntry align(1),
    tss: TssEntry align(1),
};

const GdtDescriptor = packed struct {
    limit: u16,
    base: u64,

    pub fn load(_gdt: *Gdt) GdtDescriptor {
        return .{
            .limit = @intCast(@sizeOf(Gdt) - 1),
            .base = @intFromPtr(_gdt),
        };
    }

    pub fn apply(self: *const GdtDescriptor) void {
        gdt_flush(@intFromPtr(self));
    }
};

var gdt: Gdt = std.mem.zeroes(Gdt);
var tss: Tss = std.mem.zeroes(Tss);

pub fn setup() void {
    gdt.entries[@intFromEnum(GdtType.Null)] = std.mem.zeroes(GdtEntry);

    gdt.entries[@intFromEnum(GdtType.KernelCode)] = GdtEntry.fast_init(@intFromEnum(GdtAccess.Executable), @intFromEnum(GdtFlags.LongMode));
    gdt.entries[@intFromEnum(GdtType.KernelData)] = GdtEntry.fast_init(0, @intFromEnum(GdtFlags.LongMode));
    gdt.entries[@intFromEnum(GdtType.UserData)] = GdtEntry.fast_init(@intFromEnum(GdtAccess.User), @intFromEnum(GdtFlags.LongMode));
    gdt.entries[@intFromEnum(GdtType.UserCode)] = GdtEntry.fast_init(@intFromEnum(GdtAccess.Executable) | @intFromEnum(GdtAccess.User), @intFromEnum(GdtFlags.LongMode));

    gdt.tss = TssEntry.from_addr(0);

    GdtDescriptor.load(&gdt).apply();

    tss_flush();
    log.debug("Gdt loaded", .{});
}

pub fn setup_tss() !void {
    var allocator = PageAllocator.new();
    const alloc = allocator.allocator();

    const ist0 = (try alloc.alloc(u8, kib(1))).ptr;
    const ist1 = (try alloc.alloc(u8, kib(1))).ptr;
    const rsp0 = (try alloc.alloc(u8, kib(1))).ptr;

    tss.ist.ist0 = @intFromPtr(ist0) + kib(1);
    tss.ist.ist1 = @intFromPtr(ist1) + kib(1);
    tss.rsp.rsp0 = @intFromPtr(rsp0) + kib(1);

    gdt.tss = TssEntry.from_addr(@intFromPtr(&tss));
    tss_flush();
}
