const std = @import("std");

pub fn build(b: *std.Build) void {
    // const target = b.standardTargetOptions(.{});
    // const optimize = b.standardOptimizeOption(.{});

    // const exe_mod = b.createModule(.{
    //     .target = target,
    //     .optimize = optimize,
    //     .link_libcpp = true,
    //     .link_libc = true,
    // });
    //
    // exe_mod.addIncludePath(b.path("include/"));
    // exe_mod.addLibraryPath(b.path("lib/"));
    // exe_mod.linkSystemLibrary("dpp", .{});
    //
    // exe_mod.addCSourceFiles(.{
    //     .files = &.{"src/main.cpp"},
    //     .language = .cpp,
    //     .flags = &[_][]const u8{
    //         "-std=c++20",
    //         "-Wall",
    //     },
    // });

    // const exe = b.addExecutable(.{
    //     .name = "castbort",
    //     .root_module = exe_mod,
    //     .use_llvm = true,
    //     .use_lld = true,
    // });

    const build_dir = b.addSystemCommand(&.{
        "mkdir",
        "-p",
        "build",
    });

    const exe = b.addSystemCommand(&.{
        "clang++",
        "-std=c++23",
        "-Wall",
        "-o",
        "build/castbort",
        "src/main.cpp",
        "-Llib",
        "-Iinclude",
        "-ldpp",
    });

    exe.step.dependOn(&build_dir.step);

    const build_step = b.getInstallStep();
    build_step.dependOn(&exe.step);

    const run_cmd = b.addSystemCommand(&.{
        "env",
        "LD_LIBRARY_PATH=lib",
        "build/castbort",
    });
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
    const run_step = b.step("run", "Run the application");
    run_step.dependOn(&run_cmd.step);

    const clean_step = b.step("clean", "Clean the directory");
    clean_step.dependOn(&b.addRemoveDirTree(b.path("zig-out")).step);
    clean_step.dependOn(&b.addRemoveDirTree(b.path(".zig-cache")).step);
}
