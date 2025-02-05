const sdt = @import("./sdt.zig");
const as = @import("../asm.zig");
const std = @import("std");
const lower2upper = @import("../pmm.zig").lower2upper;

const log = std.log.scoped(.hpet);

pub const Hpet = packed struct {
    pub const signature: *const [4]u8 = "HPET";

    const HpetError = error{SdtUnavailable};
    var hpet: *align(1) Hpet = undefined;
    var hpet_tick: u64 = 0;

    const Flag = struct {
        const AddressSpaceMem = 0;
        const AddressSpaceIo = 1;
        const CapCounterClockOffset = 32;
        const ConfTurnOff = 0;
        const ConfTurnOn = 1;
    };

    const Registers = struct {
        const GeneralCapabilities = 0;
        const GeneralConfiguration = 0x10;
        const MainCounterValue = 0xf0;
    };

    header: sdt.SdtHeader,

    hardware_rev_id: u8,
    info: u8,
    pci_vendor_id: u16,
    address_space_id: u8,
    register_bit_width: u8,
    register_bit_offset: u8,
    reserved1: u8,
    address: u64,
    hpet_number: u8,
    minimum_tick: u16,
    page_protection: u8,

    fn write(self: *align(1) Hpet, offset: u64, value: u64) void {
        as.writeVolatile(u64, lower2upper(self.address) + offset, value);
    }

    fn read(self: *align(1) Hpet, offset: u64) u64 {
        return as.readVolatile(u64, lower2upper(self.address) + offset);
    }

    pub fn setup() !void {
        if (sdt.sdt == null) {
            return HpetError.SdtUnavailable;
        }

        if (sdt.sdt.?.left) |rsdt| {
            hpet = try rsdt.lookup(Hpet);
        } else if (sdt.sdt.?.right) |xsdt| {
            hpet = try xsdt.lookup(Hpet);
        }

        hpet_tick = hpet.read(Registers.GeneralCapabilities) >> Flag.CapCounterClockOffset;

        hpet.write(Registers.GeneralConfiguration, Flag.ConfTurnOff);
        hpet.write(Registers.MainCounterValue, 0);
        hpet.write(Registers.GeneralConfiguration, Flag.ConfTurnOn);

        log.debug("HPET initialized", .{});
    }

    pub fn sleep(ms: usize) void {
        const target = hpet.read(Registers.MainCounterValue) + (ms * 1_000_000_000_000) / hpet_tick;
        while (hpet.read(Registers.MainCounterValue) < target) {}
        log.debug("sleep done", .{});
    }
};
