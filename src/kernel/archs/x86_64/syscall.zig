const as = @import("./asm.zig");
const GdtType = @import("./gdt.zig").GdtType;
const syscall = @import("kernel").syscall;
const Registers = @import("./regs.zig").Registers;

const navy = @import("navy");

extern fn syscall_handle() void;

const Offset = struct {
    const star_kcs_off = 32;
    const star_ucs_off = 48;
};

pub fn setup() void {
    as.writeMsr(as.Msr.efer, as.readMsr(as.Msr.efer) | 1);

    var star: u64 = 0;
    star |= (@intFromEnum(GdtType.KernelCode) * 8) << Offset.star_kcs_off;
    star |= (((@intFromEnum(GdtType.UserCode) * 8) - 0x10) | 3) << Offset.star_ucs_off;

    as.writeMsr(as.Msr.star, star);
    as.writeMsr(as.Msr.lstar, @intFromPtr(&syscall_handle));
    as.writeMsr(as.Msr.syscall_flag_mask, 0xfffffffe);
}

pub fn set_gs(addr: u64) void {
    as.writeMsr(as.Msr.gs_base, addr);
    as.writeMsr(as.Msr.kern_gs_base, addr);
}

export fn syscall_handler(regs: *Registers) callconv(.C) void {
    regs.rax = syscall.handle(regs.rax, .{
        .arg1 = regs.rdi,
        .arg2 = regs.rsi,
        .arg3 = regs.rdx,
        .arg4 = regs.r8,
        .arg5 = regs.r9,
        .arg6 = regs.r10,
    });
}
