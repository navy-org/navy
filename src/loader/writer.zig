const std = @import("std");
const uefi = std.os.uefi;
const utf16 = std.unicode.utf8ToUtf16LeStringLiteral;

pub const EfiWriter = struct {
    cout: *uefi.protocol.SimpleTextOutput,

    pub fn init() EfiWriter {
        const self: EfiWriter = .{ .cout = uefi.system_table.con_out.? };
        _ = self.cout.reset(true);
        _ = self.cout.clearScreen();
        return self;
    }

    fn writeOpaque(context: *const anyopaque, bytes: []const u8) !usize {
        const ptr: *const EfiWriter = @alignCast(@ptrCast(context));
        return ptr.write(bytes);
    }

    pub fn writer(self: *EfiWriter) std.io.AnyWriter {
        return .{ .context = self, .writeFn = writeOpaque };
    }

    pub fn write(self: EfiWriter, bytes: []const u8) usize {
        for (bytes) |b| {
            if (b == '\n') {
                _ = self.cout.outputString(utf16("\r\n"));
            } else {
                _ = self.cout.outputString(&[_:0]u16{ b, 0 });
            }
        }

        return bytes.len;
    }
};
