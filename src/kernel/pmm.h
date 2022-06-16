#pragma once 

#include <handover/handover.h>
#include <copland/base.h>

void pmm_init(Handover *handover);
RangeOption pmm_alloc(size_t size);
void pmm_free(Range page);