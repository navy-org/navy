#include "copland/debug.h"
#include <unistd.h>
#include <copland/base.h>

static void syscall_putc(MAYBE_UNUSED Writer* self, char c)
{
    char s[2] = {0};
    s[1] = c;
    syscall(SYS_DEBUG, (uintptr_t) s);
}

int main(void)
{
    Writer out = writer_init(syscall_putc);
    define_debug_out(&out);

    log$("Hello, World !");

    return 0;
}
