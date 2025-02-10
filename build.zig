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

    var target: std.Build.ResolvedTarget = undefined;
    var kernel: ?*std.Build.Step.Compile = null;

    target = b.resolveTargetQuery(arch.getKernelTarget());

    kernel = b.addExecutable(.{
        .name = "navy",
        .root_source_file = b.path("src/kernel/main.zig"),
        .target = target,
        .optimize = optimize,
        .code_model = .kernel,
    });

    const limine = b.dependency("limine", .{});

    kernel.?.root_module.addImport("loader", limine.module("limine"));
    kernel.?.want_lto = false;

    const libs = try helpers.fetchLibs(b);
    // var apps = try helpers.findModules(b, "src/apps", "main.zig");

    const servers = try helpers.findModules(b, "src/srvs", "main.zig");
    // try apps.appendSlice(servers.items);

    for (servers.items) |app| {
        const entry = b.createModule(.{ .root_source_file = b.path("src/libs/navy/entry.zig") });
        entry.resolved_target = target;
        helpers.applyModule(libs, entry);

        helpers.applyModule(libs, app);
        entry.addImport("main", app);

        const exe = b.addExecutable(.{
            .name = helpers.getFileName(b, app.root_source_file.?.dirname().src_path.sub_path),
            .root_module = entry,
        });

        exe.setLinkerScript(b.path("meta/targets/user-x86_64.ld"));
        b.installArtifact(exe);
    }

    helpers.applyModule(libs, kernel.?.root_module);
    b.installArtifact(kernel.?);
    arch.addBuildOption(b, kernel.?, libs);
}
