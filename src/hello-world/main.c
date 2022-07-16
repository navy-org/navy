#include <unistd.h>
#include <copland/base.h>

static void syscall_puts(char const *s)
{
    syscall(SYS_DEBUG, (uintptr_t) s);
}

void *realloc(void *ptr, size_t size)
{
    (void) ptr;
    (void) size;
    return NULL;
}

int _start(void)
{
    syscall_puts("Hello, World !\n");
    syscall_puts("It works !");

    loop;
    __builtin_unreachable();
}
