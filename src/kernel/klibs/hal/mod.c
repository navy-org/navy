#include "mod.h"

static size_t count = 0;

void hal_disable_interrupts(void)
{
    count++;
    if (count == 1)
    {
        _hal_disable_interrupts();
    }
}

void hal_enable_interrupts(void)
{
    if (count == 0)
    {
        return;
    }

    count--;
    if (count == 0)
    {
        _hal_enable_interrupts();
    }
}