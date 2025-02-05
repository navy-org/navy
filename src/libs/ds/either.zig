pub fn Either(Left: type, Right: type) type {
    return struct {
        left: ?Left,
        right: ?Right,
    };
}
