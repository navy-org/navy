#include <dbg/log.h>
#include <hal.h>
#include <string.h>
#include <sync/spinlock.h>

#include "gdt.h"

static Gdt gdt = {0};

static Spinlock lock = SPINLOCK_INIT;

static GdtDescriptor gdt_desc = {
    .size = sizeof(gdt) - 1,
    .offset = (uintptr_t)&gdt,
};

static void gdt_lazy_init(GdtEntry self[static 1], uint8_t access, uint8_t flags)
{
    memset(self, 0, sizeof(GdtEntry));

    if (access == 0 && flags == 0)
    {
        return;
    }

    self->access = access | GDT_ACCESS_PRESENT | GDT_ACCESS_READ_WRITE | GDT_ACCESS_DESCRIPTOR;
    self->flags = flags | GDT_FLAGS_GRANULARITY;

    self->base_high = 0;
    self->base_middle = 0;
    self->base_low = 0;

    self->limit_low = 0xffff;
    self->limit_high = 0x0f;
}

void gdt_load_tss(Tss *tss)
{
    uintptr_t tss_ptr = (uintptr_t)tss;

    spinlock_acquire(&lock);

    gdt.tss_entry = (TssEntry){
        .length = sizeof(TssEntry),

        .base_low = tss_ptr & 0xffff,
        .base_mid = (tss_ptr >> 16) & 0xff,
        .base_high = (tss_ptr >> 24) & 0xff,
        .base_upper = tss_ptr >> 32,

        .flags1 = TSS_FLAGS_PRESENT | TSS_FLAGS_64BITS_AVAILABLE,
        .flags2 = 0,

        .reserved = 0,
    };

    spinlock_release(&lock);
}

uintptr_t gdt_descriptor(void)
{
    return (uintptr_t)&gdt_desc;
}

void gdt_init(void)
{
    gdt_lazy_init(&gdt.entries[GDT_NULL_DESC], 0, 0);

    gdt_lazy_init(&gdt.entries[GDT_KERNEL_CODE], GDT_ACCESS_EXE, GDT_FLAGS_LONG_MODE);
    gdt_lazy_init(&gdt.entries[GDT_KERNEL_DATA], 0, GDT_FLAGS_SIZE);

    gdt_lazy_init(&gdt.entries[GDT_USER_DATA], GDT_ACCESS_USER, GDT_FLAGS_SIZE);
    gdt_lazy_init(&gdt.entries[GDT_USER_CODE], GDT_ACCESS_USER | GDT_ACCESS_EXE, GDT_FLAGS_LONG_MODE);
    gdt_load_tss(NULL);

    gdt_flush(gdt_descriptor());
    log$("GDT loaded");

    tss_flush();
    log$("TSS loaded");
}