#pragma once 

#include "acpi.h"

#define LAPIC_EOI           0x0b0
#define LAPIC_SPURIOUS      0x0f0
#define LAPIC_TPR           0x080
#define LAPIC_TIMER_REG     0x320
#define LAPIC_INIT          0x380
#define LAPIC_CURRENT       0x390
#define LAPIC_TIMER_DIV     0x3e0

#define LAPIC_TIMER_MASK 0x10000
#define LAPIC_TIMER_PERIODIC 0x20000

typedef struct PACKED
{
    AcpiSdt header;
    uint32_t lapic_address;
    uint32_t flags;
} Apic;

void apic_init(Handover const *handover);
void lapic_eoi(void);