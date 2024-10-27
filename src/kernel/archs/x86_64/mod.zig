pub const serial = @import("./serial.zig");
pub const endian = @import("std").builtin.Endian.little;

const gdt = @import("./gdt.zig");
const idt = @import("./idt.zig");
const limine = @import("./limine.zig");
const logger = @import("logger");
const pmm = @import("./pmm.zig");
const paging = @import("./paging.zig");
const rsdp = @import("./acpi/rsdp.zig");

const ArchError = error{LimineRsdpUnavailable};

pub fn setup() !void {
    limine.dump_mmap();
    gdt.setup();
    idt.setup();
    try pmm.setup();
    try paging.setup();

    if (limine.rsdp.response) |_rsdp| {
        _ = try rsdp.Rsdp.from_address(_rsdp.address);
    } else {
        return ArchError.LimineRsdpUnavailable;
    }
}
