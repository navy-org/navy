#include "const.h"

static uint64_t hhdm = 0;

void set_hhdm_offset(uint64_t hhdm_offset)
{
    hhdm = hhdm_offset;
}

uint64_t get_hhdm_offset(void)
{
    return hhdm;
}