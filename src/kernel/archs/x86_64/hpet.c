#include <hal.h>
#include <dbg/log.h>

#include "hpet.h"

static uintptr_t hpet_base = 0;
static size_t hpet_tick = 0;

static void hpet_write(uint32_t reg, uint64_t value)
{
    *(volatile uint64_t *)(hpet_base + reg) = value;
}

static uint64_t hpet_read(uint32_t reg)
{
    return *(volatile uint64_t *)(hpet_base + reg);
}

Res hpet_init(void)
{
    AcpiHpet *hpet = (AcpiHpet *)try$(acpi_parse_sdt("HPET"));
    hpet_base = hal_mmap_l2h(hpet->address);

    if (hpet->address_space_id == HPET_ADDRESS_SPACE_IO)
    {
        err$(RES_NOENT);
    }

    hpet_tick = hpet_read(HPET_GENERAL_CAPABILITIES) >> HPET_CAP_COUNTER_CLOCK_OFFSET;

    hpet_write(HPET_GENERAL_CONFIGUATION, HPET_CONF_TURN_OFF);
    hpet_write(HPET_MAIN_COUNTER_VALUE, 0);
    hpet_write(HPET_GENERAL_CONFIGUATION, HPET_CONF_TURN_ON);

    log$("Hpet initialised");

    return ok$();
}

void hpet_sleep(int ms)
{
    uint64_t target = hpet_read(HPET_MAIN_COUNTER_VALUE) + (ms * 1000000000000) / hpet_tick;
    while (hpet_read(HPET_MAIN_COUNTER_VALUE) <= target)
        ;
}