#include "abstraction.h"

#include <stddef.h>

static uint64_t hhdm = 0;
static void(*eoi)(void) = NULL;

void set_hhdm_offset(uint64_t hhdm_offset)
{
    hhdm = hhdm_offset;
}

uint64_t get_hhdm_offset(void)
{
    return hhdm;
}

void set_eoi_func(void (*eoi_func)(void))
{
    eoi = eoi_func;
}

void send_eoi(void)
{
    eoi();
}