#include "com.h"
#include "asm.h"

#include <kernel/spinlock.h>

static uint32_t lock;

void com_putc(Writer *self, char c)
{
    Com *com = (Com *) self;
    
    while ((asm_in8(com->port + 5) & 0x20) == 0);
    asm_out8(com->port, c);
}

void com_puts(Writer *self, char const *s)
{
    lock$(lock);
    writer_puts(self, s);
    unlock$(lock);
    
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
        .funcs = writer_init(com_putc),
        .port = port
    };
}