#pragma once 

typedef struct _Io
{
    void (*putc)(struct _Io *self, char c);
    void (*puts)(struct _Io *self, char const *s);
} Io;

void io_puts(Io *self, char const *s);
Io io_init(void (*putc)(Io *, char));