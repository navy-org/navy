const std = @import("std");
const Writer = @import("std").io.AnyWriter;

const LoggingError = error{GlobalWriterAlreadyDefined};
var global_writer: ?Writer = null;

pub fn setGlobalWriter(w: Writer) LoggingError!void {
    if (global_writer != null) {
        return LoggingError.GlobalWriterAlreadyDefined;
    }

    global_writer = w;
}

pub fn print(comptime format: []const u8, args: anytype) void {
    if (global_writer) |w| {
        w.print(format, args) catch {};
    }
}

pub fn log(
    comptime level: std.log.Level,
    comptime scope: @TypeOf(.EnumLiteral),
    comptime format: []const u8,
    args: anytype,
) void {
    if (global_writer) |w| {
        const ansi = comptime switch (level) {
            .debug => "\x1b[34m",
            .info => "\x1b[32m",
            .warn => "\x1b[33m",
            .err => "\x1b[31m",
        };

        const text = comptime switch (level) {
            .debug => "DEBUG",
            .info => "INFO",
            .warn => "WARN",
            .err => "ERROR",
        };

        const prefix = if (scope != .default) @tagName(scope) else "";

        w.print("{s}{s: >6} \x1b[90m{s: <7} \x1b[0m", .{ ansi, text, prefix }) catch {};
        w.print(format, args) catch {};
        w.print("\n", .{}) catch {};
    }
}
