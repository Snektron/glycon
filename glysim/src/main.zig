const std = @import("std");
const c = @cImport({
    @cInclude("pty.h");
});

pub fn main() !void {
    var master: std.posix.fd_t = undefined;
    var slave: std.posix.fd_t = undefined;

    // According to the openpty man page, nobody knows how large this
    // buffer should be. We need the name so that we can
    // give it to the user, so that they may connect glydb to it.
    var name: [1024]u8 = undefined;
    const result = c.openpty(&master, &slave, &name, null, null);
    std.log.debug("result: {}", .{result});
    std.log.debug("name: {s}", .{std.mem.sliceTo(&name, 0)});

    defer {
        std.posix.close(master);
        std.posix.close(slave);
    }

    var tty = std.fs.File{.handle = master};
    const reader = tty.reader();

    var buf: [1024]u8 = undefined;
    while (true) {
        const nread = try reader.read(&buf);
        std.log.debug("nread: {}", .{nread});
    }
}
