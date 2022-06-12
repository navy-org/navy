#include <limine>
#include <copland>

static struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

int _start(void)
{
    (void) terminal_request;
    loop;
}