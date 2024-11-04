pub const MapFlag = struct {
    pub const none: u8 = 1 << 0;
    pub const read: u8 = 1 << 1;
    pub const write: u8 = 1 << 2;
    pub const execute: u8 = 1 << 3;
    pub const user: u8 = 1 << 4;
    pub const huge: u8 = 1 << 5;
};
