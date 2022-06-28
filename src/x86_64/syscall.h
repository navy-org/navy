#pragma once

#include "base.h"

extern void syscall_handle(void);
void syscall_init(void);


typedef int64_t (*Syscall)(Regs *);