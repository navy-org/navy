#pragma once 

#include "asm.h"
#include "com.h"
#include "rtc.h"
#include "regs.h"
#include "vmm.h"

#include <handover/handover.h>

void hardware_init(Handover const *handover);