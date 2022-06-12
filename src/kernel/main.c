#include <limine>
#include <copland>
#include <x86_64>

static struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

int _start(void)
{
    (void) terminal_request;

    Com debug = com_init(COM1);
    debug.funcs.puts((Io *) &debug, "Hello, World !");
    loop;
}