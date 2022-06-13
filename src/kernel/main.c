#include <limine/limine.h>
#include <x86_64/com.h>
#include <copland/base.h>

static struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

int _start(void)
{
    (void) terminal_request;

    Com debug = com_init(COM1);
    define_debug_out((Writer *) &debug);
    log$("Hello, World !");

    loop;
}