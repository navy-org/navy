#include <stdint.h>

#define PAGE_SIZE 0x1000

void set_hhdm_offset(uint64_t hhdm_offset);
uint64_t get_hhdm_offset(void);