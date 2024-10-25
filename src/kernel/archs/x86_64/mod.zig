pub const serial = @import("./serial.zig");

const gdt = @import("./gdt.zig");
const idt = @import("./idt.zig");
const limine = @import("./limine.zig");
const logger = @import("logger");
const pmm = @import("./pmm.zig");
const paging = @import("./paging.zig");

pub fn setup() !void {
    limine.dump_mmap(logger.debug);
    gdt.setup();
    idt.setup();
    try pmm.setup();
    try paging.setup();
}
