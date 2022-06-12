#include <x86_64>

void com_putc(Io *self, char c)
{
    Com *com = (Com *) self;
    
    while ((asm_in8(com->port + 5) & 0x20) == 0);
    asm_out8(com->port, c);
}

Com com_init(ComPort port)
{
    asm_out8(port + 1, 0x00);
    asm_out8(port + 0, 0x03);
    asm_out8(port + 1, 0x00);
    asm_out8(port + 3, 0x03);
    asm_out8(port + 2, 0xc7);
    asm_out8(port + 4, 0x0b);
    asm_out8(port + 4, 0x1e);
    asm_out8(port + 0, 0xae);
    asm_out8(port + 4, 0x0f);

    return (Com) {
        .funcs = io_init(com_putc),
        .port = port
    };
}