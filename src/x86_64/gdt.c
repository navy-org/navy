#include "gdt.h"
#include "const.h"

#include <kernel/spinlock.h>

#include <copland/debug.h>
#include <stdlib.h>

static Gdt gdt;
static Tss tss = {0};
static uint32_t lock;

static GdtPointer gdtDescriptor = {
    .limit = sizeof(Gdt) - 1,
    .base = (uintptr_t) &gdt
};

static void init_descriptor(GdtEntry *self, uint32_t base, uint32_t limit, uint8_t access)
{
    if (access != GDT_DATA)
    {
        self->flags = 0b0010;
    }
    else
    {
        self->flags = 0;
    }

    self->access = 0b10000010 | access;

    self->base_low = base & 0xffff;
    self->base_mid = (base & 0xff0000) >> 16;
    self->base_high = (base & 0xff000000) >> 24;

    self->limit_low = limit & 0xffff;
    self->limit_high = (limit & 0xf0000) >> 16;
}

static TssEntry init_tss(uintptr_t self)
{
    return (TssEntry)
    {
        .length = sizeof(TssEntry),
        .base_low16 = (uint16_t) self & 0xffff,
        .base_mid8 = (uint8_t) (self >> 16 ) & 0xff,
        .flags1 = 0b10001001,
        .flags2 = 0,
        .base_high8 = (uint8_t) (self >> 24) & 0xff,
        .base_upper32 = self >> 32,
        .reserved = 0,
    };
}

void gdt_init(void)
{
    
    init_descriptor(&gdt.entries[GDT_NULL], 0, 0, 0);

    init_descriptor(&gdt.entries[GDT_KERNEL_CODE], 0, 0, GDT_KERNEL | GDT_CODE);
    init_descriptor(&gdt.entries[GDT_KERNEL_DATA], 0, 0, GDT_KERNEL | GDT_DATA);

    init_descriptor(&gdt.entries[GDT_USER_DATA], 0, 0, GDT_USER | GDT_DATA);
    init_descriptor(&gdt.entries[GDT_USER_CODE], 0, 0, GDT_USER | GDT_CODE);

    gdt.tss = init_tss((uintptr_t) &tss);
    tss.iopb = sizeof(Tss);

    gdt_flush((uintptr_t) &gdtDescriptor);
}

void gdt_load_tss(Tss *tss)
{
    lock$(lock);
    gdt.tss = init_tss((uintptr_t) tss);

    tss_flush();
    unlock$(lock);
}