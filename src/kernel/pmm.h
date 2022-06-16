#pragma once 

#include <handover/handover.h>
#include <copland/base.h>

void pmm_init(Handover *handover);
typedef Option(Range) PmmOption;
PmmOption pmm_alloc(size_t size);
void pmm_free(Range page);