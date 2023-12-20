#pragma once

#include <hal.h>
#include <res.h>

Res load_scheduler(void);

void switch_to_scheduler(HalRegs *regs);