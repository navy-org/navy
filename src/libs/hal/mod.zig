pub const MapFlag = enum(u8) {
    None = 1 << 0,
    Read = 1 << 1,
    Write = 1 << 2,
    Execute = 1 << 3,
    User = 1 << 4,
    Huge = 1 << 5,
};
