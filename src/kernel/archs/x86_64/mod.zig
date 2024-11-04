pub const endian = std.builtin.Endian.little;
pub const serial = @import("./serial.zig");
pub const as = @import("./asm.zig");

const std = @import("std");

const apic = @import("./acpi/apic.zig");
const gdt = @import("./gdt.zig");
const idt = @import("./idt.zig");
const limine = @import("./limine.zig");
const logger = @import("logger");
const madt = @import("./acpi/madt.zig");
const paging = @import("./paging.zig");
const Hpet = @import("./acpi/hpet.zig").Hpet;
const pmm = @import("./pmm.zig");
const rsdp = @import("./acpi/rsdp.zig");

const ArchError = error{LimineRsdpUnavailable};

pub fn setup() !void {
    limine.dumpMmap();
    gdt.setup();
    idt.setup();
    try pmm.setup();
    try paging.setup();

    if (limine.rsdp.response) |r| {
        const _rsdp = try rsdp.Rsdp.fromMem(r.address);
        _ = try _rsdp.findSdt();
        const m = try madt.Madt.setup();

        try Hpet.setup();

        const lapic = apic.Lapic.fromAddress(pmm.lower2upper(m.localApicAddr));
        lapic.init();
        try lapic.initTimer();
        try apic.Ioapic.redirectLegacyIrq();
    } else {
        return ArchError.LimineRsdpUnavailable;
    }
}
