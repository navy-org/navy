const std = @import("std");
const helpers = @import("../helpers.zig");

pub fn getKernelTarget() std.Target.Query {
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

pub fn addBuildOption(b: *std.Build, kernel: *std.Build.Step.Compile, modules: std.StringHashMap(*std.Build.Module)) void {
    const arch = b.createModule(.{ .root_source_file = b.path("src/kernel/archs/x86_64/mod.zig") });
    arch.addAssemblyFile(b.path("src/kernel/archs/x86_64/helper.s"));

    arch.addImport("kernel", kernel.root_module);
    kernel.root_module.addImport("arch", arch);

    helpers.applyModule(modules, arch);

    kernel.setLinkerScript(b.path("meta/targets/kernel-x86_64.ld"));
}
