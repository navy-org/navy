#pragma once 

#include "asm.h"
#include "com.h"
#include "rtc.h"
#include "regs.h"
#include "vmm.h"
#include "pic.h"
#include "pit.h"
#include "acpi.h"
#include "madt.h"

#include <handover/handover.h>

void hardware_init(Handover const *handover);