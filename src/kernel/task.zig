const std = @import("std");
const pmm = @import("arch").pmm;
const MapFlags = @import("hal").MapFlag;
var serial = @import("root").serial;

const PageAllocator = pmm.PageAllocator;
const upper2lower = pmm.upper2lower;

const AnyCap = @import("./capability.zig").AnyCap;
const Channel = @import("./channel.zig").Channel;
const Space = @import("arch").paging.Space;
const Context = @import("arch").context.Context;
const Elf = @import("elf").Elf;
const Spinlock = @import("sync").Spinlock;
const TaskMM = @import("./task_mm.zig").TaskMM;

const log = std.log.scoped(.task);

pub const Task = struct {
    name: []const u8,
    pid: u32,
    space: Space,
    ctx: *Context,
    caps: std.ArrayList(AnyCap),
    mm: TaskMM,

    stack: std.ArrayList([*:0]const u8),
    phys_stack: [*]u8,

    var gpa = std.heap.GeneralPurposeAllocator(.{ .MutexType = Spinlock }){};
    const alloc = gpa.allocator();

    var allocator = PageAllocator.new();
    const palloc = allocator.allocator();

    pub fn from_elf(name: []const u8, elf: Elf, args: []const []const u8) !*Task {
        var space = try Space.create();

        for (elf.phdr[0..elf.header.e_phnum]) |phentry| {
            if (phentry.p_type == std.elf.PT_LOAD) {
                log.debug("{s} - loading segment between 0x{x:0>16} & 0x{x:0>16}", .{ name, phentry.p_vaddr, phentry.p_vaddr + phentry.p_memsz });

                const alignedSize = std.mem.alignForward(usize, phentry.p_memsz, std.heap.pageSize());
                const sz = phentry.p_filesz;
                const page: [*]u8 = @ptrCast(try palloc.alloc(u8, alignedSize));

                try space.map(phentry.p_vaddr, upper2lower(@intFromPtr(page)), alignedSize, MapFlags.read | MapFlags.write | MapFlags.execute | MapFlags.user);

                std.mem.copyForwards(u8, page[0..sz], elf.raw[phentry.p_offset .. phentry.p_offset + sz]);
                @memset(page[phentry.p_filesz..phentry.p_memsz], 0);
            }
        }

        return try new(name, elf.header.e_entry, space, true, args);
    }

    pub fn addArgRaw(self: *Task, addr: usize) !void {
        try self.stack.append(@ptrFromInt(addr - @intFromPtr(self.phys_stack) + Context.STACK_BASE));
        self.ctx.regs.rdi += 1;
    }

    pub fn addArgZ(self: *Task, arg: []const u8) !void {
        const argZ = try self.stack.allocator.dupeZ(u8, arg);
        try self.addArgRaw(@intFromPtr(argZ.ptr));
    }

    pub fn addArg(self: *Task, arg: []const u8) !void {
        const argp = try self.stack.allocator.dupe(u8, arg);
        try self.addArgRaw(@intFromPtr(argp.ptr));
    }

    pub fn new(name: []const u8, ip: usize, space: Space, need_stack: bool, args: []const []const u8) !*Task {
        log.debug("Creating {s}, ip: {x}, space: {x}", .{ name, ip, @intFromPtr(space.root) });

        var self = try alloc.create(Task);
        var ctx = try alloc.create(Context);
        var argp: usize = 0;

        if (need_stack) {
            self.phys_stack = (try palloc.alloc(u8, Context.STACK_SIZE)).ptr;
            @memset(self.phys_stack[0..Context.STACK_SIZE], 0);
            try space.map(Context.STACK_BASE, upper2lower(@intFromPtr(self.phys_stack)), Context.STACK_SIZE, MapFlags.user | MapFlags.read | MapFlags.write);

            var stack_allocator: std.heap.FixedBufferAllocator = .init(self.phys_stack[0..Context.STACK_SIZE]);
            const stack_alloc = stack_allocator.allocator();
            self.stack = .init(stack_alloc);

            const nameArg = try stack_alloc.dupeZ(u8, name);
            try self.stack.append(@ptrFromInt(@intFromPtr(nameArg.ptr) - @intFromPtr(self.phys_stack) + Context.STACK_BASE));

            for (args) |arg| {
                const argZ = try stack_alloc.dupeZ(u8, arg);
                try self.stack.append(@ptrFromInt(@intFromPtr(argZ.ptr) - @intFromPtr(self.phys_stack) + Context.STACK_BASE));
            }

            const nameZ = try alloc.dupeZ(u8, name);
            self.mm = .init(nameZ);
            argp = @intFromPtr(self.stack.items.ptr) - @intFromPtr(self.phys_stack) + Context.STACK_BASE;
        }

        try ctx.setup(ip, Context.STACK_TOP, args.len + 1, argp);

        self.name = name;
        self.ctx = ctx;
        self.space = space;
        self.caps = std.ArrayList(AnyCap).init(alloc);
        try self.caps.append(serial.capability());

        return self;
    }
};
