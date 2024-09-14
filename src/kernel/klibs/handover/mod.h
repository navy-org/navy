#pragma once

#define HANDOVER_INCLUDE_UTILITES

#include "handover.h"
#include "utils.h"

/* --- Handover ------------------------------------------------------------ */

HandoverPayload *handover(void);

/* --- Misc ---------------------------------------------------------------- */

typedef struct
{
    size_t phys;
    size_t virt;
} KernelMmap;

KernelMmap loader_get_kernel_mmap(void);