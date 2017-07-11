const Builder = @import("std").build.Builder;
const builtin = @import("builtin");

pub fn build(b: &Builder) {
    const exe = b.addCExecutable("faststack");

    exe.addCompileFlags([][]const u8 {
        "-Wall", "-Wextra", "-std=c99", "-O2", "-pedantic",
        "-DFS_USE_TERMINAL", "-Isrc/engine"
    });

    const source_files = [][]const u8 {
        "src/engine/control.c",
        "src/engine/engine.c",
        "src/engine/finesse.c",
        "src/engine/fslibc.c",
        "src/engine/hiscore.c",
        "src/engine/keyspec.c",
        "src/engine/log.c",
        "src/engine/option.c",
        "src/engine/rand.c",
        "src/engine/replay.c",
        "src/engine/rotation.c",

        "src/frontend/terminal/main.c",
        "src/frontend/terminal/frontend.c",
        "src/frontend/terminal/glyph.c",
    };

    for (source_files) |source| {
        exe.addSourceFile(source);
    }

    exe.setOutputPath("./faststack");
    b.default_step.dependOn(&exe.step);
}
