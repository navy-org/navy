#pragma once 

#include <handover/handover.h>
#include <copland/base.h>

typedef Result(Handover, Str) ResultHandover;

ResultHandover handover_create(void);