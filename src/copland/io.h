#pragma once 

typedef struct _Writer 
{
    void (*putc)(struct _Writer *self, char c);
    void (*puts)(struct _Writer *self, char const *s);
} Writer;

void writer_puts(Writer *self, char const *s);
Writer writer_init(void (*putc)(Writer *, char));