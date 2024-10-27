const std = @import("std");

pub const Symbols = struct {
    const Self = @This();

    dwarf: std.dwarf.DwarfInfo,
    alloc: std.mem.Allocator,

    pub fn from_elf(elf: Elf, endian: std.builtin.Endian, alloc: std.mem.Allocator) !Self {
        return .{
            .dwarf = try elf.getDwarf(endian, alloc),
            .alloc = alloc,
        };
    }

    pub fn deinit(self: *Self) void {
        self.dwarf.deinit(self.alloc);
    }

    const Info = struct {
        lineInfo: std.debug.LineInfo,
        symbol: []const u8,
    };

    pub fn infoForAddr(self: *Self, addr: usize) !Info {
        const compile_unit = try self.dwarf.findCompileUnit(addr);
        return .{
            .lineInfo = try self.dwarf.getLineNumberInfo(self.alloc, compile_unit.*, addr),
            .symbol = self.dwarf.getSymbolName(addr) orelse "unknown",
        };
    }
};

pub const Elf = struct {
    const Self = @This();
    header: *std.elf.Ehdr,
    shtab: [*]std.elf.Shdr,
    shstrtab: *std.elf.Shdr,

    pub fn fromSlice(data: [*]u8) Self {
        const header: *std.elf.Ehdr = @alignCast(@ptrCast(data));
        const shtab: [*]std.elf.Shdr = @alignCast(@ptrCast(data + header.e_shoff));

        return .{
            .header = header,
            .shtab = shtab,
            .shstrtab = &shtab[header.e_shstrndx],
        };
    }

    pub fn getSectionByName(self: Self, name: []const u8) ?*std.elf.Shdr {
        const shdr_str: [*]const u8 = @ptrFromInt(@intFromPtr(self.header) + self.shstrtab.sh_offset);
        for (0..self.header.e_shnum) |i| {
            const section: *std.elf.Shdr = &self.shtab[i];
            const section_name: [*]const u8 = @ptrFromInt(@intFromPtr(shdr_str) + section.sh_name);
            if (std.mem.eql(u8, section_name[0..name.len], name)) {
                return section;
            }
        }
        return null;
    }

    pub fn getSectionData(self: Self, section: *std.elf.Shdr) []u8 {
        const data: [*]u8 = @ptrFromInt(@intFromPtr(self.header) + section.sh_offset);
        return data[0..section.sh_size];
    }

    pub fn getSectionDataByName(self: Self, name: []const u8) ?[]u8 {
        const section = self.getSectionByName(name);
        if (section == null) {
            return null;
        }
        return self.getSectionData(section.?);
    }

    pub fn getDwarf(self: Self, endian: std.builtin.Endian, alloc: std.mem.Allocator) !std.dwarf.DwarfInfo {
        var dwarf = std.dwarf.DwarfInfo{
            .endian = endian,
            .sections = .{
                .{ .data = self.getSectionDataByName(".debug_info") orelse &[1]u8{0}, .owned = true },
                .{ .data = self.getSectionDataByName(".debug_abbrev") orelse &[1]u8{0}, .owned = true },
                .{ .data = self.getSectionDataByName(".debug_str") orelse &[1]u8{0}, .owned = true },
                null, // debug_str_offsets
                .{ .data = self.getSectionDataByName(".debug_line") orelse &[1]u8{0}, .owned = true },
                null, // debug_line_str
                .{ .data = self.getSectionDataByName(".debug_ranges") orelse &[1]u8{0}, .owned = true },
                null, // debug_loclists
                null, // debug_rnglists
                null, // debug_addr
                null, // debug_names
                null, // debug_frame
                null, // eh_frame
                null, // eh_frame_hdr
            },
            .is_macho = false,
        };

        try std.dwarf.openDwarfDebugInfo(&dwarf, alloc);

        return dwarf;
    }
};
