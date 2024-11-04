const SdtHeader = @import("./sdt.zig").SdtHeader;
const lower2upper = @import("../pmm.zig").lower2upper;
const apic = @import("./apic.zig");
const sdt = @import("./sdt.zig");

const std = @import("std");

var madt: *align(1) Madt = undefined;
const MadtError = error{SdtUnavailable};

pub fn getMadt() *align(1) Madt {
    return madt;
}

pub const HeaderEntry = extern struct {
    type: u8,
    length: u8,
};

pub const Iso = extern struct {
    header: HeaderEntry,
    bus: u8,
    irq: u8,
    gsi: u32,
    flags: u16,
};

pub const Madt = packed struct {
    const Self = @This();

    const EntryTypes = struct {
        const Lapic: u8 = 0;
        const Ioapic: u8 = 1;
        const Iso: u8 = 2;
        const NonMaskableSrc: u8 = 3;
        const NonMaskableInt: u8 = 4;
        const LapicOverride: u8 = 5;
        const Lapic2x: u8 = 9;
    };

    pub const signature: *const [4]u8 = "APIC";

    header: SdtHeader,
    localApicAddr: u32,
    flags: u32,
    entries: void,

    pub fn lapic(self: *align(1) Self) apic.Lapic {
        return apic.Lapic.fromAddress(lower2upper(self.localApicAddr));
    }

    pub fn getIsoFromIrq(self: *align(1) Self, irq: u8) ?*align(1) Iso {
        var i: usize = 0;

        while (i < self.header.length - @sizeOf(Self)) {
            const entry: *align(1) HeaderEntry = @ptrFromInt(@intFromPtr(&self.entries) + i);

            if (entry.type == Self.EntryTypes.Iso) {
                const iso: *align(1) Iso = @ptrCast(entry);
                if (iso.irq == irq) {
                    return iso;
                }
            }

            i += @max(2, entry.length);
        }

        return null;
    }

    pub fn getIoApicFromGsi(self: *align(1) Self, gsi: u32) ?*align(1) apic.Ioapic {
        var i: usize = 0;

        while (i < self.header.length - @sizeOf(Self)) {
            const entry: *align(1) HeaderEntry = @ptrFromInt(@intFromPtr(&self.entries) + i);

            if (entry.type == Self.EntryTypes.Ioapic) {
                const ioapic: *align(1) apic.Ioapic = @ptrCast(entry);
                if (ioapic.gsiBase <= gsi and gsi < ioapic.gsiBase + ioapic.countGsi()) {
                    return ioapic;
                }
            }

            i += @max(2, entry.length);
        }

        return null;
    }

    pub fn setup() !*align(1) Self {
        if (sdt.sdt == null) {
            return MadtError.SdtUnavailable;
        }
        if (sdt.sdt.?.left) |rsdt| {
            madt = try rsdt.lookup(Madt);
        } else if (sdt.sdt.?.right) |xsdt| {
            madt = try xsdt.lookup(Madt);
        }

        return madt;
    }
};
