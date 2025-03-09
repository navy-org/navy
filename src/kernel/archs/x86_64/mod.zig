pub const endian = std.builtin.Endian.little;
pub const serial = @import("./serial.zig");
pub const as = @import("./asm.zig");
pub const pmm = @import("./pmm.zig");
pub const paging = @import("./paging.zig");
pub const context = @import("./ctx.zig");
pub const syscall = @import("./syscall.zig");
pub const loader = @import("./limine.zig");

const std = @import("std");

const utils = @import("utils");
const apic = @import("./acpi/apic.zig");
const gdt = @import("./gdt.zig");
const idt = @import("./idt.zig");
const logger = @import("logger");
const madt = @import("./acpi/madt.zig");
const Hpet = @import("./acpi/hpet.zig").Hpet;
const rsdp = @import("./acpi/rsdp.zig");

const ArchError = error{LimineRsdpUnavailable};

// NOTE: 1Gib doesn't work for some reason.
pub const page_size_max = utils.mib(2);
pub const page_size_min = utils.kib(4);

pub fn setup() !void {
    loader.dumpMmap();
    gdt.setup();
    idt.setup();
    try pmm.setup();
    try gdt.setup_tss();
    try paging.setup();
    syscall.setup();

    if (loader.rsdp.response) |r| {
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
