const std = @import("std");
const uefi = std.os.uefi;

const utils = @import("utils");
const mem = @import("./mem.zig");
const MapFlag = @import("hal").MapFlag;
const EfiPageAllocator = mem.EfiPageAllocator;

var image: ?*uefi.protocol.LoadedImage = null;
var rootfs: ?*uefi.protocol.SimpleFileSystem = null;
var rootdir: ?*uefi.protocol.File = null;

const log = std.log.scoped(.loader);

pub fn efiImage() !*uefi.protocol.LoadedImage {
    return image orelse {
        if (uefi.system_table.boot_services) |bs| {
            image = try bs.openProtocolSt(uefi.protocol.LoadedImage, uefi.handle);
        }

        return image.?;
    };
}

pub fn efiRootfs() !*uefi.protocol.SimpleFileSystem {
    return rootfs orelse {
        if (uefi.system_table.boot_services) |bs| {
            const img = try efiImage();
            rootfs = try bs.openProtocolSt(uefi.protocol.SimpleFileSystem, img.device_handle.?);
        }

        return rootfs.?;
    };
}

pub fn efiRootdir() !*uefi.protocol.File {
    return rootdir orelse {
        const fs = try efiRootfs();
        var rd: *uefi.protocol.File = undefined;
        try fs.openVolume(&rd).err();
        rootdir = rd;
        return rootdir.?;
    };
}

pub fn efiOpenFile(path: []const u8) !*uefi.protocol.File {
    log.debug("Opening file {s}", .{path});
    var filePath: [256:0]u16 = std.mem.zeroes([256:0]u16);

    for (0..path.len, path) |i, c| {
        if (c == '/') {
            filePath[i] = '\\';
        } else {
            filePath[i] = @intCast(c);
        }
    }

    const dir = try efiRootdir();
    var file: *uefi.protocol.File = undefined;

    try dir.open(&file, &filePath, uefi.protocol.File.efi_file_mode_read, uefi.protocol.File.efi_file_read_only).err();
    return file;
}

// --- Elf loader --------------------------------------------------------------

pub fn loadElf(filename: []const u8) !usize {
    var allocator = EfiPageAllocator.new();
    const alloc = allocator.allocator();

    const file = try efiOpenFile(filename);
    defer _ = file.close();

    const header = try std.elf.Header.read(file);
    var pHeaders = header.program_header_iterator(file);

    while (try pHeaders.next()) |phdr| {
        if (phdr.p_type == std.elf.PT_LOAD) {
            log.debug("Loading segment at 0x{x:0>16} < 0x{x:0>16}", .{ phdr.p_vaddr, phdr.p_vaddr + phdr.p_memsz });
            const alignedSize = std.mem.alignForward(usize, phdr.p_memsz, std.mem.page_size);
            var sz = phdr.p_filesz;
            const page: [*]u8 = @ptrCast(try alloc.alloc(u8, alignedSize));

            const delta: usize = phdr.p_vaddr - std.mem.alignBackward(usize, phdr.p_vaddr, std.mem.page_size);

            log.debug("Page at 0x{x:0>16} < 0x{x:0>16} (len: {x})", .{ @intFromPtr(page), @intFromPtr(page) + alignedSize, alignedSize });

            try file.setPosition(phdr.p_offset).err();
            log.debug("Reading with delta {x}", .{delta});
            try file.read(&sz, @ptrCast(page)).err();

            try mem.space.map(phdr.p_vaddr, @intFromPtr(page), alignedSize, MapFlag.read | MapFlag.write | MapFlag.execute);
            @memset(page[phdr.p_filesz..phdr.p_memsz], 0);
        }
    }

    return header.entry;
}
