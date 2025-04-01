const std = @import("std");
const log = std.log.scoped(.ctx);
const mib = @import("utils").mib;
const kib = @import("utils").kib;
const PageAlloc = @import("./pmm.zig").PageAllocator;
const upper2lower = @import("./pmm.zig").upper2lower;
const syscall = @import("./syscall.zig");

const GdtType = @import("./gdt.zig").GdtType;
pub const Registers = @import("./regs.zig").Registers;

pub const Rflags = struct {
    const carry: u64 = 1 << 0;
    const reserved1: u64 = 1 << 1;
    const parity: u64 = 1 << 2;
    const reserved2: u64 = 1 << 3;
    const auxiliary_carry: u64 = 1 << 4;
    const reserved3: u64 = 1 << 5;
    const zero: u64 = 1 << 6;
    const sign: u64 = 1 << 7;
    const trap: u64 = 1 << 8;
    const interrupt_enable: u64 = 1 << 9;
    const direction: u64 = 1 << 10;
    const overflow: u64 = 1 << 11;
    const io_privilege_level: u64 = 1 << 12;
    const nested_task: u64 = 1 << 14;
    const reserved4: u64 = 1 << 15;
    const _resume: u64 = 1 << 16;
    const virtual_8086_mode: u64 = 1 << 17;
    const alignment_check: u64 = 1 << 18;
    const virtual_interrupt: u64 = 1 << 19;
    const virtual_interrupt_pending: u64 = 1 << 20;
    const id: u64 = 1 << 21;
    const reserved5: u64 = 1 << 22;
};

pub const Context = packed struct {
    pub const STACK_SIZE = mib(2);
    pub const STACK_TOP = 0x7fffffffe000;
    pub const STACK_BASE = (STACK_TOP - STACK_SIZE);

    var allocator = PageAlloc.new();
    const alloc = allocator.allocator();

    syscall_kernel_stack: usize,
    syscall_user_stack: usize,

    regs: Registers,

    pub fn save(self: *Context, regs: *Registers) void {
        self.regs = regs.*;
    }

    pub fn restore(self: *Context, regs: *Registers) void {
        regs.* = self.regs;
        syscall.set_gs(@intFromPtr(self));
    }

    pub fn setup(self: *Context, ip: u64, sp: u64, argc: u64, argp: u64) !void {
        self.regs = std.mem.zeroes(Registers);

        const krnl_stck = try alloc.alloc(u8, STACK_SIZE);
        self.syscall_kernel_stack = @intFromPtr(krnl_stck.ptr) + STACK_SIZE;

        self.regs.rip = ip;
        self.regs.rflags = Rflags.interrupt_enable | Rflags.reserved1;
        self.regs.cs = (@intFromEnum(GdtType.UserCode) * 8) | 3;
        self.regs.ss = (@intFromEnum(GdtType.UserData) * 8) | 3;
        self.regs.rsp = sp;
        self.regs.rdi = argc;
        self.regs.rsi = argp;
        self.regs.rbp = 0;
    }
};
