#include <hal>
#include <logger>

#include "apic.h"
#include "asm.h"
#include "hpet.h"

static Madt *madt = NULL;

/* --- Lapic --------------------------------------------------------------- */

static uint32_t lapic_read(uint32_t reg)
{
    return *((volatile uint32_t *)(hal_mmap_l2h(madt->local_controller_address) + reg));
}

static void lapic_write(uint32_t reg, uint32_t value)
{
    *((volatile uint32_t *)(hal_mmap_l2h(madt->local_controller_address) + reg)) = value;
}

void lapic_timer_start(void)
{
    lapic_write(LAPIC_REG_TIMER_DIV, APIC_TIMER_DIVIDE_BY_16);
    lapic_write(LAPIC_REG_TIMER_INITCNT, 0xFFFFFFFF);

    hpet_sleep(10);

    lapic_write(LAPIC_REG_LVT_TIMER, LAPIC_TIMER_MASKED);

    uint32_t tick_in_10ms = 0xFFFFFFFF - lapic_read(LAPIC_REG_TIMER_CURRCNT);

    lapic_write(LAPIC_REG_LVT_TIMER, LAPIC_TIMER_IRQ | LAPIC_TIMER_PERIODIC);
    lapic_write(LAPIC_REG_TIMER_DIV, APIC_TIMER_DIVIDE_BY_16);
    lapic_write(LAPIC_REG_TIMER_INITCNT, tick_in_10ms / 10);
}

static void lapic_enable(void)
{
    asm_write_msr(MSR_APIC, (asm_read_msr(MSR_APIC) | LAPIC_ENABLE) & ~((1 << 10)));
    lapic_write(LAPIC_REG_SPURIOUS, lapic_read(LAPIC_REG_SPURIOUS) | (LAPIC_SPURIOUS_ALL | LAPIC_SPURIOUS_ENABLE_APIC));

    lapic_timer_start();
}

void lapic_eoi(void)
{
    if (madt == NULL)
    {
        return;
    }

    lapic_write(LAPIC_REG_EOI, 0);
}

int lapic_id(void)
{
    if (madt == NULL)
    {
        return -1;
    }

    return lapic_read(LAPIC_CPU_ID) >> 24;
}

/* --- Ioapic --------------------------------------------------------------- */

static void ioapic_write(MadtIoapic *io_apic, uint32_t reg, uint32_t value)
{
    uintptr_t base = (uintptr_t)hal_mmap_l2h(io_apic->ioapic_addr);
    *(volatile uint32_t *)base = reg;
    *(volatile uint32_t *)(base + 16) = value;
}

static uint32_t ioapic_read(MadtIoapic *ioapic, uint32_t reg)
{
    uintptr_t base = (uintptr_t)hal_mmap_l2h(ioapic->ioapic_addr);
    *(volatile uint32_t *)(base) = reg;
    return *(volatile uint32_t *)(base + 0x10);
}

static void ioapic_redirect_legacy(void)
{
    for (size_t i = 0; i < 16; i++)
    {
        ioapic_redirect_irq(0, i + 32, i);
    }
}

static MadtIso *madt_get_iso_irq(uint8_t irq)
{
    size_t i = 0;
    while (i < madt->header.length - sizeof(Madt))
    {
        MadtEntry *entry = (MadtEntry *)madt->entries + i;

        if (entry->type == 2)
        {
            MadtIso *iso = (MadtIso *)entry;
            if (iso->irq_src == irq)
            {
                return iso;
            }
        }

        i += max$(2, entry->length);
    }

    return NULL;
}

static size_t ioapic_gsi_count(MadtIoapic *ioapic)
{
    uint32_t val = ioapic_read(ioapic, 1);
    IoapicVer *ver = (IoapicVer *)&val;
    return ver->max_redirect;
}

MadtIoapic *madt_get_ioapic_from_gsi(uint32_t gsi)
{
    size_t i = 0;
    MadtEntry *entry;
    while (i < madt->header.length - sizeof(Madt))
    {
        entry = (MadtEntry *)(madt->entries + i);

        if (entry->type == 1)
        {
            MadtIoapic *ioapic = (MadtIoapic *)entry;

            if (gsi >= ioapic->gsib && gsi < ioapic->gsib + ioapic_gsi_count(ioapic))
            {
                return ioapic;
            }
        }

        i += max$(2, entry->length);
    }

    return NULL;
}

static void ioapic_set_gsi_redirect(uint32_t lapic_id, uint8_t intno, uint8_t gsi, uint16_t flags)
{
    uint32_t io_redirect_table;
    IoapicRedirect redirect = {0};
    MadtIoapic *ioapic = madt_get_ioapic_from_gsi(gsi);

    if (ioapic == NULL)
    {
        return;
    }

    redirect.vector = intno;

    if (flags & IOAPIC_ACTIVE_HIGH_LOW)
    {
        redirect.polarity = 1;
    }

    if (flags & IOAPIC_TRIGGER_EDGE_LOW)
    {
        redirect.trigger = 1;
    }

    redirect.dest_id = lapic_id;

    io_redirect_table = (gsi - ioapic->gsib) * 2 + 16;
    ioapic_write(ioapic, io_redirect_table, (uint32_t)redirect._raw.low_byte);
    ioapic_write(ioapic, io_redirect_table + 1, (uint32_t)redirect._raw.high_byte);
}

void ioapic_redirect_irq(uint32_t lapic_id, uint8_t intno, uint8_t irq)
{
    MadtIso *iso = madt_get_iso_irq(irq);
    if (iso != NULL)
    {
        ioapic_set_gsi_redirect(lapic_id, intno, iso->gsi, iso->flags);
    }
    else
    {
        ioapic_set_gsi_redirect(lapic_id, intno, irq, 0);
    }
}

Res apic_init(void)
{
    madt = (Madt *)try$(acpi_parse_sdt("APIC"));
    lapic_enable();
    ioapic_redirect_legacy();

    hal_enable_interrupts();

    log$("APIC initialised");

    return ok$();
}
