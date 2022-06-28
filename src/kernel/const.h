#include <stdint.h>

#define PAGE_SIZE 0x1000
#define STACK_SIZE 0x4000
#define USER_STACK_BASE 0xc0000000

void set_hhdm_offset(uint64_t hhdm_offset);
uint64_t get_hhdm_offset(void);