const std = @import("std");
const LazyPath = std.build.LazyPath;

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const glyco = blk: {
        // Build coprocessor image.
        // Note: this doesn't run on the host, but on an arduino 'coprocessor' board.
        // Currently, Zig is not very good at compiling AVR, because it goes through
        // LLVM. For now, shell out to avr-gcc instead of building via the zig cc
        // functions.
        const object = b.addSystemCommand(&.{
            "avr-gcc",
            "-ffunction-sections",
            "-fdata-sections",
            "-mmcu=atmega2560",
            "-DF_CPU=16000000L",
            "-DARDUINO=10815",
            "-DARDUINO_AVR_MEGA2560",
            "-DARDUINO_ARCH_AVR",
            "-DBAUD=1000000UL",
            "-DBAUD_TOL=3",
            "-Os",
        });
        object.addFileArg(b.path("glyco/src/bus.c"));
        object.addFileArg(b.path("glyco/src/flash.c"));
        object.addFileArg(b.path("glyco/src/main.c"));
        object.addFileArg(b.path("glyco/src/serial.c"));
        object.addPrefixedDirectoryArg("-I", b.path("glyco/src"));
        object.addPrefixedDirectoryArg("-I", b.path("common/include"));
        const elf = object.addPrefixedOutputFileArg("-o", "glyco.elf");

        const bin = b.addObjCopy(elf, .{
            .only_section = ".text",
            .format = .bin,
        });

        b.getInstallStep().dependOn(&b.addInstallFile(bin.getOutput(), "share/glycon/glyco.bin").step);

        break :blk bin.getOutput();
    };

    {
        // Flash utility command
        const flash_port = b.option([]const u8, "flash-port", "Set the path to the ttyUSB for the coprocessor") orelse "/dev/ttyUSB0";
        const flash = b.addSystemCommand(&.{
            "avrdude",
            "-p",
            "atmega2560",
            "-c",
            "wiring",
            "-b",
            "115200",
            "-D",
            "-P",
            flash_port,
            "-U",
        });
        flash.addPrefixedFileArg("flash:w:", glyco);

        const flash_step = b.step("flash", "Flash glyco to the coprocessor");
        flash_step.dependOn(&flash.step);
    }

    {
        // Build debugger
        const glydb = b.addExecutable(.{
            .name = "glydb",
            .link_libc = true,
            .target = target,
            .optimize = optimize,
        });
        glydb.addCSourceFiles(.{
            .root = b.path("glydb/src"),
            .files = &.{
                "bdbp_util.c",
                "buffer.c",
                "command.c",
                "connection.c",
                "debugger.c",
                "main.c",
                "parser.c",
                "target.c",
                "value.c",
                "commands/commands.c",
                "commands/connection.c",
                "commands/disassemble.c",
                "commands/flash.c",
                "commands/help.c",
                "commands/memory.c",
                "commands/ping.c",
                "commands/quit.c",
                "z80/disassemble.c",
                "z80/z80.c",
            },
            .flags = &.{
                "-std=gnu11",
            },
        });
        glydb.linkSystemLibrary("editline");
        glydb.addIncludePath(b.path("common/include"));
        glydb.addIncludePath(b.path("glydb/src"));
        b.installArtifact(glydb);
    }

    {
        // Build OS
        const glyos_assemble = b.addSystemCommand(&.{ "scas", "-o" });
        const glyos = glyos_assemble.addOutputFileArg("kernel.bin");
        glyos_assemble.addFileArg(b.path("glyos/kernel/main.z80"));
        b.getInstallStep().dependOn(&b.addInstallFile(glyos, "share/glycon/kernel.bin").step);
    }

    {
        // Build simulator
        const glysim = b.addExecutable(.{
            .name = "glysim",
            .root_source_file = b.path("glysim/src/main.zig"),
            .target = target,
            .optimize = optimize,
            .link_libc = true,
        });
        b.installArtifact(glysim);
    }
}
