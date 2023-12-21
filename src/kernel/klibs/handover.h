#pragma once

#include <handover/handover.h>
#include <handover/builder.h>
#include <handover/utils.h>

/* --- Handover ------------------------------------------------------------ */

HandoverPayload *handover(void);

void handover_parse_module(HandoverBuilder *self);

void handover_parse_mmap(HandoverBuilder *self);

/* --- Misc ---------------------------------------------------------------- */

typedef struct
{
    size_t phys;
    size_t virt;
} KernelMmap;

KernelMmap loader_get_kernel_mmap(void);