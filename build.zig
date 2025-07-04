const std = @import("std");
const helpers = @import("./meta/helpers.zig");

const buildError = error{
    InvalidArch,
};

pub fn build(b: *std.Build) !void {
    const cross_target_spec = b.option(std.Target.Cpu.Arch, "arch", "Target Architecture") orelse std.Target.Cpu.Arch.x86_64;
    const optimize = b.standardOptimizeOption(.{});
    const arch = switch (cross_target_spec) {
        std.Target.Cpu.Arch.x86_64 => @import("meta/targets/kernel-x86_64.zig"),
        else => return error.InvalidArch,
    };

    const target = b.resolveTargetQuery(arch.getKernelTarget());
    const kernel = b.addExecutable(.{
        .name = "navy",
        .root_source_file = b.path("src/kernel/main.zig"),
        .target = target,
        .optimize = optimize,
        .code_model = .kernel,
    });

    const tinyvmem = b.createModule(.{ .root_source_file = b.path("./src/libs/mem/vmem/vmem.zig") });
    tinyvmem.addIncludePath(b.path("./src/libs/mem/vmem"));
    tinyvmem.addCSourceFile(.{ .file = b.path("./src/libs/mem/vmem/vmem.c") });

    const limine = b.dependency("limine", .{
        .api_revision = 3,
        .allow_deprecated = false,
        .no_pointers = false,
    });
    kernel.root_module.addImport("loader", limine.module("limine"));
    kernel.want_lto = false;

    var liblist = try helpers.findModules(b, "src/libs/", "mod.zig");
    defer liblist.deinit();

    const libs = try helpers.mkLibs(b, liblist);

    var srvlist = try helpers.findModules(b, "src/srvs/", "lib.zig");
    defer srvlist.deinit();

    for (srvlist.items) |lib| {
        helpers.applyModules(libs, lib);
    }

    helpers.applyModules(libs, tinyvmem);
    kernel.root_module.addImport("vmem", tinyvmem);

    const srvlibs = try helpers.mkLibs(b, srvlist);

    var apps = try helpers.findModules(b, "src/apps", "main.zig");
    const servers = try helpers.findModules(b, "src/srvs", "main.zig");

    try apps.appendSlice(servers.items);
    defer servers.deinit();
    defer apps.deinit();

    for (servers.items) |app| {
        const entry = b.createModule(.{ .root_source_file = b.path("src/libs/navy/entry.zig") });
        entry.resolved_target = target;

        helpers.applyModules(libs, entry);
        helpers.applyModules(libs, app);
        helpers.applyModules(srvlibs, app);

        entry.addImport("main", app);

        const exe = b.addExecutable(.{
            .name = helpers.getFileName(b, app.root_source_file.?.dirname().src_path.sub_path),
            .root_module = entry,
        });

        exe.setLinkerScript(b.path("meta/targets/user-x86_64.ld"));
        b.installArtifact(exe);
    }

    helpers.applyModules(libs, kernel.root_module);
    b.installArtifact(kernel);
    arch.addBuildOption(b, kernel, libs);
}
