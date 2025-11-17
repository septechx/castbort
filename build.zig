const std = @import("std");

const ExecutableOptions = struct {
    name: []const u8,
    source_files: []const []const u8,
    include_dirs: []const []const u8,
    library_dirs: []const []const u8,
    libraries: []const []const u8,
};

fn addExecutable(b: *std.Build, comptime options: ExecutableOptions) *std.Build.Step.Run {
    var cmd = std.ArrayList([]const u8).initCapacity(b.allocator, options.source_files.len +
        options.include_dirs.len + options.library_dirs.len + options.libraries.len + 5) catch @panic("OOM");
    defer cmd.deinit(b.allocator);
    cmd.appendSlice(b.allocator, &.{
        "clang++",
        "-std=c++23",
        "-Wall",
        "-o",
        "build/" ++ options.name,
    }) catch @panic("OOM");
    for (options.source_files) |source_file| {
        cmd.appendSlice(b.allocator, &.{
            source_file,
        }) catch @panic("OOM");
    }
    for (options.include_dirs) |include_dir| {
        cmd.appendSlice(b.allocator, &.{
            "-I",
            include_dir,
        }) catch @panic("OOM");
    }
    for (options.library_dirs) |library_dir| {
        cmd.appendSlice(b.allocator, &.{
            "-L",
            library_dir,
        }) catch @panic("OOM");
    }
    for (options.libraries) |library| {
        cmd.appendSlice(b.allocator, &.{
            "-l",
            library,
        }) catch @panic("OOM");
    }
    const step = b.addSystemCommand(cmd.items);
    return step;
}

pub fn build(b: *std.Build) void {
    const build_dir = b.addSystemCommand(&.{ "mkdir", "-p", "build" });

    const exe = addExecutable(b, .{
        .name = "castbort",
        .include_dirs = &.{"include"},
        .library_dirs = &.{"lib"},
        .source_files = &.{
            "src/main.cpp",
            "src/database.cpp",
        },
        .libraries = &.{
            "dpp",
            "sqlite3",
        },
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
