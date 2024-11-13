const std = @import("std");

pub const efiFilename = "bootx64";
pub const efiTarget = std.Target.Query{
    .cpu_arch = .x86_64,
    .os_tag = .uefi,
    .abi = .msvc,
};

pub fn getBuildTarget() std.Target.Query {
    var target: std.Target.Query = .{ .cpu_arch = .x86_64, .os_tag = .freestanding, .abi = .none };

    const Features = std.Target.x86.Feature;
    target.cpu_features_sub.addFeature(@intFromEnum(Features.mmx));
    target.cpu_features_sub.addFeature(@intFromEnum(Features.sse));
    target.cpu_features_sub.addFeature(@intFromEnum(Features.sse2));
    target.cpu_features_sub.addFeature(@intFromEnum(Features.avx));
    target.cpu_features_sub.addFeature(@intFromEnum(Features.avx2));
    target.cpu_features_add.addFeature(@intFromEnum(Features.soft_float));

    return target;
}

pub fn addBuildOption(b: *std.Build, kernel: *std.Build.Step.Compile, loader: *std.Build.Step.Compile, modules: std.StringHashMap(*std.Build.Module)) void {
    const arch = b.createModule(.{ .root_source_file = b.path("src/kernel/archs/x86_64/mod.zig") });
    arch.addAssemblyFile(b.path("src/kernel/archs/x86_64/helper.s"));

    const archEfi = b.createModule(.{ .root_source_file = b.path("src/kernel/archs/x86_64/paging.zig") });

    kernel.root_module.addImport("arch", arch);
    loader.root_module.addImport("arch", archEfi);

    var it = modules.keyIterator();

    while (it.next()) |key| {
        const module = modules.get(key.*);
        arch.addImport(key.*, module.?);
        archEfi.addImport(key.*, module.?);
    }

    kernel.setLinkerScriptPath(b.path("meta/targets/kernel-x86_64.ld"));
}
