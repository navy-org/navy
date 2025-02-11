const std = @import("std");

pub fn findModules(b: *std.Build, starting_path: []const u8, filename: []const u8) !std.ArrayList(*std.Build.Module) {
    var liblist = std.ArrayList(*std.Build.Module).init(b.allocator);

    var dir = try std.fs.cwd().openDir(starting_path, .{ .iterate = true });
    defer dir.close();

    var walker = try dir.walk(b.allocator);
    defer walker.deinit();

    while (try walker.next()) |entry| {
        if (std.mem.indexOf(u8, entry.path, filename) != null) {
            const path = try std.fmt.allocPrint(b.allocator, "{s}/{s}", .{ starting_path, entry.path });
            try liblist.append(b.createModule(.{ .root_source_file = b.path(path) }));
        }
    }

    return liblist;
}

pub fn getFileName(b: *std.Build, path: []const u8) []const u8 {
    const pathBuf = b.allocator.dupe(u8, path) catch "";
    std.mem.reverse(u8, @constCast(pathBuf));

    for (0..path.len, pathBuf) |i, char| {
        if (char == '/') {
            return path[(path.len - i)..];
        }
    }

    return pathBuf;
}

pub fn mkLibs(b: *std.Build, modules: std.ArrayList(*std.Build.Module)) !std.StringHashMap(*std.Build.Module) {
    var map = std.StringHashMap(*std.Build.Module).init(b.allocator);

    for (modules.items) |lib| {
        const libname = getFileName(b, lib.root_source_file.?.dirname().src_path.sub_path);
        try map.put(libname, lib);
    }

    return map;
}

pub fn applyModules(modules: std.StringHashMap(*std.Build.Module), module: *std.Build.Module) void {
    var it = modules.keyIterator();
    while (it.next()) |key| {
        const mod = modules.get(key.*);
        module.addImport(key.*, mod.?);
    }
}
