#include "base.h"
#include "x86_64/acpi.h"
#include "x86_64/asm.h"
#include <copland/base.h>

static uintptr_t lapic_addr;

static uint32_t lapic_read(uint32_t reg)
{
    return *((volatile uint32_t *)(lapic_addr + reg));
}

static void lapic_write(uint32_t reg, uint32_t value)
{
    *((volatile uint32_t *)(lapic_addr + reg)) = value;
}

static void lapic_init(void)
{
    asm_write_msr(MSR_APIC, (asm_read_msr(MSR_APIC) | LAPIC_ENABLE) & ~((1 << 10)));
    lapic_write(LAPIC_SPURIOUS, lapic_read(LAPIC_SPURIOUS) | 0x1ff);
}

static void lapic_timer_init(void)
{
    lapic_write(LAPIC_TPR, 0);

    lapic_write(LAPIC_TIMER_DIV, 3);
    lapic_write(LAPIC_INIT, 0xffffffff);

    pit_sleep(10);

    lapic_write(LAPIC_TIMER_REG, LAPIC_TIMER_MASK);

    uint32_t ticks = 0xffffffff - lapic_read(LAPIC_CURRENT);

    lapic_write(LAPIC_TIMER_REG, 32 | LAPIC_TIMER_PERIODIC);
    lapic_write(LAPIC_TIMER_DIV, 3);
    lapic_write(LAPIC_INIT, ticks);
}

void apic_init(Handover const *handover)
{
    Madt *madt = (Madt *) UNWRAP(acpi_parse(str$("APIC")));
    acpi_checksum((AcpiSdt *) madt);

    lapic_addr = madt->lapic_address + handover->hhdm_offset;
    lapic_init();
    lapic_timer_init();
}

void lapic_eoi(void)
{
    lapic_write(LAPIC_EOI, 0);
}