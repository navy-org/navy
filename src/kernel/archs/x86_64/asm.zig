const std = @import("std");

pub fn in8(port: u16) u8 {
    return asm volatile ("inb %[port],%[ret]"
        : [ret] "={al}" (-> u8),
        : [port] "N{dx}" (port),
    );
}

pub fn out8(port: u16, value: u8) void {
    asm volatile ("outb %[value],%[port]"
        :
        : [value] "{al}" (value),
          [port] "N{dx}" (port),
    );
}

const Cr = struct {
    number: u8,

    pub fn read(self: Cr) u64 {
        var ret: u64 = 0;

        switch (self.number) {
            0 => asm volatile ("mov %%cr0, %[ret]"
                : [ret] "={rax}" (ret),
                :
                : "rax"
            ),
            2 => asm volatile ("mov %%cr2, %[ret]"
                : [ret] "={rax}" (ret),
                :
                : "rax"
            ),
            3 => asm volatile ("mov %%cr3, %[ret]"
                : [ret] "={rax}" (ret),
                :
                : "rax"
            ),
            4 => asm volatile ("mov %%cr4, %[ret]"
                : [ret] "={rax}" (ret),
                :
                : "rax"
            ),
            else => unreachable,
        }

        return ret;
    }

    pub fn write(self: Cr, value: u64) void {
        switch (self.number) {
            0 => asm volatile ("mov %[value], %%cr0"
                :
                : [value] "r" (value),
                : "memory"
            ),
            2 => asm volatile ("mov %[value], %%cr2"
                :
                : [value] "r" (value),
                : "memory"
            ),
            3 => asm volatile ("mov %[value], %%cr3"
                :
                : [value] "r" (value),
                : "memory"
            ),
            4 => asm volatile ("mov %[value], %%cr4"
                :
                : [value] "r" (value),
                : "memory"
            ),
            else => unreachable,
        }
    }
};

pub const cr0 = Cr{ .number = 0 };
pub const cr2 = Cr{ .number = 2 };
pub const cr3 = Cr{ .number = 3 };
pub const cr4 = Cr{ .number = 4 };

pub const Msr = struct {
    pub const apic = 0x1B;
    pub const efer = 0xC0000080;
    pub const star = 0xC0000081;
    pub const lstar = 0xC0000082;
    pub const compat_star = 0xC0000083;
    pub const syscall_flag_mask = 0xC0000084;
    pub const fs_base = 0xC0000100;
    pub const gs_base = 0xC0000101;
    pub const kern_gs_base = 0xc0000102;
};

pub fn writeMsr(msr: u64, value: u64) void {
    const low: u32 = @truncate(value);
    const high: u32 = @truncate(value >> 32);

    asm volatile ("wrmsr"
        :
        : [_] "{rcx}" (msr),
          [_] "{eax}" (low),
          [_] "{edx}" (high),
    );
}

pub fn readMsr(msr: u64) u64 {
    var low: u32 = 0;
    var high: u32 = 0;

    asm volatile ("rdmsr"
        : [low] "={eax}" (low),
          [high] "={edx}" (high),
        : [msr] "{rcx}" (msr),
    );

    return @as(u64, @intCast(high)) << 32 | low;
}

pub fn readVolatile(T: type, addr: usize) T {
    return @as(*align(1) volatile T, @ptrFromInt(addr)).*;
}

pub fn writeVolatile(T: type, addr: usize, value: T) void {
    @as(*align(1) volatile T, @ptrFromInt(addr)).* = value;
}

pub fn pause() void {
    asm volatile ("pause");
}

pub fn hlt() noreturn {
    while (true) {
        asm volatile ("hlt");
    }
}

pub fn disableInterrupts() void {
    asm volatile ("cli");
}

pub fn enableInterrupts() void {
    asm volatile ("sti");
}
