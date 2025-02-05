const as = @import("../asm.zig");
const maxInt = @import("std").math.maxInt;
const Hpet = @import("./hpet.zig").Hpet;
const std = @import("std");
const madt = @import("./madt.zig");
const lower2upper = @import("../pmm.zig").lower2upper;

const log = std.log.scoped(.apic);

const apicMsr = 0x1b;

pub const Lapic = struct {
    const LapicError = error{MadtUnavailable};

    const msrEnable = 0x800;

    const spuriousAll = 0xff;
    const spuriousEnableApic = 0x100;
    const timerMasked = 0x10000;
    const timerIrq = 32;
    const timerPeriodic = 0x20000;

    const Registers = struct {
        const cpuId: u64 = 0x020;
        const eoi: u64 = 0x0b0;
        const spurious: u64 = 0x0f0;
        const icr0: u64 = 0x300;
        const icr1: u64 = 0x310;
        const lvtTimer: u64 = 0x320;
        const timerInitCnt: u64 = 0x380;
        const timerCurrCnt: u64 = 0x390;
        const timerDiv: u64 = 0x3e0;
    };

    const Divisor = struct {
        const _1 = 7;
        const _2 = 0;
        const _4 = 1;
        const _8 = 2;
        const _16 = 3;
        const _32 = 4;
        const _64 = 5;
        const _128 = 6;
    };

    addr: usize,

    pub fn read(self: Lapic, reg: u64) u32 {
        return as.readVolatile(u32, self.addr + reg);
    }

    pub fn write(self: Lapic, reg: u64, value: u32) void {
        as.writeVolatile(u32, self.addr + reg, value);
    }

    pub fn fromAddress(address: usize) Lapic {
        return .{
            .addr = address,
        };
    }

    pub fn init(self: Lapic) void {
        as.writeMsr(apicMsr, as.readMsr(apicMsr) | msrEnable & ~@as(u64, 1 << 10));

        self.write(
            Lapic.Registers.spurious,
            self.read(Lapic.Registers.spurious) | (Lapic.spuriousAll | Lapic.spuriousEnableApic),
        );

        log.debug("Lapic initialized", .{});
    }

    pub fn initTimer(self: Lapic) !void {
        self.write(Lapic.Registers.timerDiv, Lapic.Divisor._16);
        self.write(Lapic.Registers.timerInitCnt, maxInt(u32));

        Hpet.sleep(10);

        self.write(Lapic.Registers.lvtTimer, Lapic.timerMasked);
        const ticks = maxInt(u32) - self.read(Lapic.Registers.timerCurrCnt);

        self.write(Lapic.Registers.lvtTimer, Lapic.timerIrq | Lapic.timerPeriodic);
        self.write(Lapic.Registers.timerDiv, Lapic.Divisor._16);
        self.write(Lapic.Registers.timerInitCnt, ticks / 10);

        as.enableInterrupts();

        log.debug("Lapic timer initialized", .{});
    }

    pub fn eoi() void {
        madt.getMadt().lapic().write(Lapic.Registers.eoi, 0);
    }
};

pub const Ioapic = extern struct {
    const IoapicError = error{
        IoapicUnavailable,
    };

    header: madt.HeaderEntry,
    id: u8,
    reversed: u8,
    addr: u32,
    gsiBase: u32,

    const Flags = struct {
        const ActiveHighLow = 1 << 1;
        const TriggerEdgeLow = 1 << 3;
    };

    const IoapicRedirect = packed union {
        redirect: packed struct {
            vector: u8,
            deliveryMode: u3,
            destMode: u1,
            deliveryStatus: u1,
            polarity: u1,
            remote_irr: u1,
            trigger: u1,
            mask: u1,
            reserved: u7,
            destination: u8,
        },

        _raw: packed struct {
            low: u32,
            high: u32,
        },
    };

    const IoapicVersion = packed struct {
        version: u8,
        reserved: u8,
        max_redir: u8,
        reserved2: u8,
    };

    fn write(self: *align(1) Ioapic, reg: u32, value: u32) void {
        const base = lower2upper(self.addr);
        as.writeVolatile(u32, base, reg);
        as.writeVolatile(u32, base + 0x10, value);
    }

    fn read(self: *align(1) Ioapic, reg: u32) u32 {
        const base = lower2upper(self.addr);
        as.writeVolatile(u32, base, reg);
        return as.readVolatile(u32, base + 0x10);
    }

    fn setGsiRedirect(lapic_id: u32, intno: u8, gsi: u8, flags: u16) !void {
        const ioapic = madt.getMadt().getIoApicFromGsi(gsi);
        var redirect: IoapicRedirect = .{ ._raw = .{ .low = 0, .high = 0 } };

        if (ioapic) |i| {
            redirect.redirect.vector = intno;

            if (flags & Ioapic.Flags.ActiveHighLow == Ioapic.Flags.ActiveHighLow) {
                redirect.redirect.polarity = 1;
            }

            if (flags & Ioapic.Flags.TriggerEdgeLow == Ioapic.Flags.TriggerEdgeLow) {
                redirect.redirect.trigger = 1;
            }

            redirect.redirect.destination = @as(u8, @intCast(lapic_id));

            const io_redirect_table: u32 = (gsi - i.gsiBase) * 2 + 0x10;
            i.write(io_redirect_table, redirect._raw.low);
            i.write(io_redirect_table + 1, redirect._raw.high);
        } else {
            return IoapicError.IoapicUnavailable;
        }
    }

    pub fn countGsi(self: *align(1) Ioapic) usize {
        var val = self.read(1);
        const ver: *align(1) IoapicVersion = @ptrCast(&val);
        return @intCast(ver.max_redir);
    }

    pub fn redirectIrq(lapic_id: u32, intno: u8, irq: u8) !void {
        const iso = madt.getMadt().getIsoFromIrq(irq);
        if (iso) |i| {
            try Ioapic.setGsiRedirect(lapic_id, intno, @as(u8, @intCast(i.gsi)), i.flags);
        } else {
            try Ioapic.setGsiRedirect(lapic_id, intno, irq, 0);
        }
    }

    pub fn redirectLegacyIrq() !void {
        for (0..16) |i| {
            try Ioapic.redirectIrq(0, @as(u8, @intCast(i)) + 32, @as(u8, @intCast(i)));
        }
    }
};
