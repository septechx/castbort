const std = @import("std");

pub fn build(b: *std.Build) void {
    const optimize = b.standardOptimizeOption(.{});

    const build_dir = b.addSystemCommand(&.{ "mkdir", "-p", "build" });

    const exe = addExecutable(b, .{
        .name = "castbort",
        .include_dirs = &.{"include"},
        .library_dirs = &.{"lib"},
        .source_files = &.{
            "src/main.cpp",
            "src/database.cpp",
            "src/video_generator.cpp",
        },
        .libraries = &.{
            "dpp",
            "sqlite3",
        },
        .optimize = optimize,
    });

    exe.step.dependOn(&build_dir.step);

    const build_step = b.getInstallStep();
    build_step.dependOn(&exe.step);

    const run_cmd = b.addSystemCommand(&.{ "env", "LD_LIBRARY_PATH=lib", "build/castbort" });
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

const ExecutableOptions = struct {
    name: []const u8,
    optimize: std.builtin.OptimizeMode = std.builtin.OptimizeMode.Debug,
    source_files: []const []const u8 = &.{},
    include_dirs: []const []const u8 = &.{},
    library_dirs: []const []const u8 = &.{},
    libraries: []const []const u8 = &.{},
};

fn addExecutable(b: *std.Build, options: ExecutableOptions) *std.Build.Step.Run {
    var cmd = std.ArrayList([]const u8).initCapacity(b.allocator, options.source_files.len +
        options.include_dirs.len * 2 + options.library_dirs.len * 2 + options.libraries.len * 2 + 6) catch @panic("OOM");
    defer cmd.deinit(b.allocator);
    const mode = switch (options.optimize) {
        .Debug => "-O0",
        .ReleaseFast => "-O3",
        .ReleaseSmall => "-Oz",
        .ReleaseSafe => @panic("ReleaseSafe is not supported"),
    };
    const output_file = b.fmt("build/{s}", .{options.name});
    defer b.allocator.free(output_file);
    cmd.appendSliceAssumeCapacity(&.{
        "clang++",
        "-std=c++23",
        "-Wall",
        "-o",
        output_file,
        mode,
    });
    for (options.source_files) |source_file| {
        cmd.appendSliceAssumeCapacity(&.{
            source_file,
        });
    }
    for (options.include_dirs) |include_dir| {
        cmd.appendSliceAssumeCapacity(&.{
            "-I",
            include_dir,
        });
    }
    for (options.library_dirs) |library_dir| {
        cmd.appendSliceAssumeCapacity(&.{
            "-L",
            library_dir,
        });
    }
    for (options.libraries) |library| {
        cmd.appendSliceAssumeCapacity(&.{
            "-l",
            library,
        });
    }
    return b.addSystemCommand(cmd.items);
}
