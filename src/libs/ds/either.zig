pub fn Either(Left: type, Right: type) type {
    return struct {
        const Self = @This();

        left: ?Left,
        right: ?Right,
    };
}
