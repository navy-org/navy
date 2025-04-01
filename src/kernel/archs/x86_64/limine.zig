pub const impl = @import("root").loader;
const std = @import("std");

pub export var base_revision: impl.BaseRevision = .{ .revision = 3 };
pub export var hhdm: impl.HhdmRequest = .{};
pub export var mmap: impl.MemoryMapRequest = .{};
pub export var kaddr: impl.KernelAddressRequest = .{};
pub export var rsdp: impl.RsdpRequest = .{};
pub export var kernel: impl.KernelFileRequest = .{};
pub export var modules: impl.ModuleRequest = .{};

const log = std.log.scoped(.limine);

pub fn dumpMmap() void {
    if (mmap.response) |m| {
        log.debug("+-------------------------------------------------------------------+", .{});
        log.debug("|{s: ^24} | {s: ^18} | {s: ^18} |", .{ "Type", "Base", "Limit" });
        log.debug("+-------------------------------------------------------------------+", .{});
        for (0..m.entry_count) |i| {
            const entry = m.entries()[i];
            log.debug("|{s: ^24} | 0x{x:0>16} | 0x{x:0>16} |", .{ std.enums.tagName(impl.MemoryMapEntryType, entry.kind).?, entry.base, entry.base + entry.length });
        }
        log.debug("+-------------------------------------------------------------------+", .{});
    }
}

pub fn findFile(path: []const u8) ?*impl.File {
    if (modules.response) |mods| {
        for (mods.modules()) |mod| {
            if (std.mem.eql(u8, path, mod.path[0..path.len])) {
                return mod;
            }
        }
    }

    return null;
}

pub fn getFilesMapping() ?[]*impl.File {
    if (modules.response) |mods| {
        return mods.modules();
    }

    return null;
}
