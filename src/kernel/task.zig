const std = @import("std");
const Elf = @import("elf").Elf;
const MapFlags = @import("hal").MapFlag;
const Spinlock = @import("sync").Spinlock;

const AnyCap = @import("./capability.zig").AnyCap;
const pmm = @import("arch").pmm;
const PageAllocator = pmm.PageAllocator;
const upper2lower = pmm.upper2lower;

const Space = @import("arch").paging.Space;
const Context = @import("arch").context.Context;

const log = std.log.scoped(.task);
var serial = @import("root").serial;

pub const Task = struct {
    name: []const u8,
    pid: u32,
    space: Space,
    ctx: *Context,
    caps: std.ArrayList(AnyCap),

    var gpa = std.heap.GeneralPurposeAllocator(.{ .MutexType = Spinlock }){};
    const alloc = gpa.allocator();

    var allocator = PageAllocator.new();
    const palloc = allocator.allocator();

    pub fn from_elf(name: []const u8, elf: Elf) !*Task {
        var space = try Space.create();

        for (elf.phdr[0..elf.header.e_phnum]) |phentry| {
            if (phentry.p_type == std.elf.PT_LOAD) {
                log.debug("{s} - loading segment between 0x{x:0>16} & 0x{x:0>16}", .{ name, phentry.p_vaddr, phentry.p_vaddr + phentry.p_memsz });

                const alignedSize = std.mem.alignForward(usize, phentry.p_memsz, std.mem.page_size);
                const sz = phentry.p_filesz;
                const page: [*]u8 = @ptrCast(try palloc.alloc(u8, alignedSize));

                try space.map(phentry.p_vaddr, upper2lower(@intFromPtr(page)), alignedSize, MapFlags.read | MapFlags.write | MapFlags.execute | MapFlags.user);

                std.mem.copyForwards(u8, page[0..sz], elf.raw[phentry.p_offset .. phentry.p_offset + sz]);
                @memset(page[phentry.p_filesz..phentry.p_memsz], 0);
            }
        }

        return try new(name, elf.header.e_entry, space, true);
    }

    pub fn new(name: []const u8, ip: usize, space: Space, need_stack: bool) !*Task {
        log.debug("Creating {s}, ip: {x}, space: {x}", .{ name, ip, @intFromPtr(space.root) });

        var self = try alloc.create(Task);
        var ctx = try alloc.create(Context);

        if (need_stack) {
            var page: [*]u8 = (try palloc.alloc(u8, Context.STACK_SIZE)).ptr;
            @memset(page[0..Context.STACK_SIZE], 0);
            try space.map(Context.STACK_BASE, upper2lower(@intFromPtr(page)), Context.STACK_SIZE, MapFlags.user | MapFlags.read | MapFlags.write);
        }

        try ctx.setup(ip, Context.STACK_TOP);

        self.name = name;
        self.ctx = ctx;
        self.space = space;
        self.caps = std.ArrayList(AnyCap).init(alloc);
        try self.caps.append(serial.capability());

        return self;
    }
};
