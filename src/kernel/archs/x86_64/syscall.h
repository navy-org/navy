#pragma once

#include <stdint.h>

void syscall_handle(void);

void syscall_set_gs(uintptr_t addr);

void syscall_init(void);
