const std = @import("std");

pub const Symbols = struct {
    dwarf: std.debug.Dwarf,
    alloc: std.mem.Allocator,

    pub fn from_elf(elf: Elf, endian: std.builtin.Endian, alloc: std.mem.Allocator) !Symbols {
        return .{
            .dwarf = try elf.getDwarf(endian, alloc),
            .alloc = alloc,
        };
    }

    pub fn deinit(self: *Symbols) void {
        self.dwarf.deinit(self.alloc);
    }

    const Info = struct {
        lineInfo: std.debug.SourceLocation,
        symbol: []const u8,
    };

    pub fn infoForAddr(self: *Symbols, addr: usize) !Info {
        const compile_unit = try self.dwarf.findCompileUnit(addr);
        return .{
            .lineInfo = try self.dwarf.getLineNumberInfo(self.alloc, compile_unit, addr),
            .symbol = self.dwarf.getSymbolName(addr) orelse "unknown",
        };
    }
};

pub const Elf = struct {
    header: *std.elf.Ehdr,
    shtab: [*]std.elf.Shdr,
    shstrtab: *std.elf.Shdr,

    pub fn fromSlice(data: [*]u8) Elf {
        const header: *std.elf.Ehdr = @alignCast(@ptrCast(data));
        const shtab: [*]std.elf.Shdr = @alignCast(@ptrCast(data + header.e_shoff));

        return .{
            .header = header,
            .shtab = shtab,
            .shstrtab = &shtab[header.e_shstrndx],
        };
    }

    pub fn getSectionByName(self: Elf, name: []const u8) ?*std.elf.Shdr {
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

    pub fn getSectionData(self: Elf, section: *std.elf.Shdr) []u8 {
        const data: [*]u8 = @ptrFromInt(@intFromPtr(self.header) + section.sh_offset);
        return data[0..section.sh_size];
    }

    pub fn getSectionDataByName(self: Elf, name: []const u8) ?[]u8 {
        const section = self.getSectionByName(name);
        if (section == null) {
            return null;
        }
        return self.getSectionData(section.?);
    }

    pub fn getDwarf(self: Elf, endian: std.builtin.Endian, alloc: std.mem.Allocator) !std.debug.Dwarf {
        var dwarf = std.debug.Dwarf{
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

        try dwarf.open(alloc);

        return dwarf;
    }
};
