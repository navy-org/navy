#include <copland>

void io_puts(Io *self, char const *s)
{
    while (*s)
    {
        self->putc(self, *s++);
    }
}

Io io_init(void (*putc)(Io *, char))
{
    return (Io) {
        .putc = putc,
        .puts = io_puts
    };
}