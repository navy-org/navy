#pragma once 

#include <stdint.h>

#ifdef __x86_64__
#include <x86_64/base.h>
#endif

void set_hhdm_offset(uint64_t hhdm_offset);
uint64_t get_hhdm_offset(void);

void set_eoi_func(void (*eoi_func)(void));
void send_eoi(void);
