#pragma once

#define max$(a, b) (((a) > (b)) ? (a) : (b))

#define min$(a, b) (((a) < (b)) ? (a) : (b))

#define make_enum$(enum) enum,

#define make_str$(str) #str,


#define kib$(x) ((uintptr_t)(x)*1024)

#define mib$(x) (kib$(x) * 1024)

#define gib$(x) (mib$(x) * 1024)

#define align_up$(x, align)   (((x) + (align)-1) & ~((align)-1))

#define align_down$(x, align) ((x) & ~((align)-1))