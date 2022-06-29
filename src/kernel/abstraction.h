#pragma once 

#if defined(__osdk_arch_x86__) && defined(__osdk_sub_64__)
#include <x86_64/base.h>
#endif

#ifdef __osdk_sched_round_robin__
#include <round-robin/base.h>
#endif

void set_hhdm_offset(uint64_t hhdm_offset);
uint64_t get_hhdm_offset(void);

void set_eoi_func(void (*eoi_func)(void));
void send_eoi(void);