#pragma once

#define asm_read_cr(n, reg) asm volatile("mov %%cr" #n ", %0" \
                                         : "=r"(reg))

#define asm_write_cr(n, reg) asm volatile("mov %0, %%cr" #n \
                                          :                 \
                                          : "r"(reg))