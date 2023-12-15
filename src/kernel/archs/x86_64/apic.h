#pragma once

#include "acpi.h"

#define IPI_RESCHED                (100)
#define IPI_STOP                   (101)
#define LAPIC_ENABLE               (0x800)
#define LAPIC_SPURIOUS_ALL         0xff
#define LAPIC_SPURIOUS_ENABLE_APIC 0x100
#define LAPIC_ICR_CPUID_OFFSET     24
#define LAPIC_ICR_CLEAR_INIT_LEVEL (1 << 14)
#define LAPIC_ICR_DEST_INIT        (5 << 8)
#define LAPIC_ICR_DEST_SEND_IPI    (6 << 8)
#define IOAPIC_REG_OFFSET          (0)
#define IOAPIC_VALUE_OFFSET        (16)
#define LAPIC_TIMER_IRQ            (32)
#define LAPIC_TIMER_PERIODIC       (0x20000)
#define LAPIC_TIMER_MASKED         (0x10000)
#define IOAPIC_ACTIVE_HIGH_LOW     (1 << 1)
#define IOAPIC_TRIGGER_EDGE_LOW    (1 << 3)

enum lapic_reg
{
    LAPIC_CPU_ID = 0x20,
    LAPIC_REG_EOI = 0x0b0,
    LAPIC_REG_SPURIOUS = 0x0f0,
    LAPIC_REG_ICR0 = 0x300,
    LAPIC_REG_ICR1 = 0x310,
    LAPIC_REG_LVT_TIMER = 0x320,
    LAPIC_REG_TIMER_INITCNT = 0x380,
    LAPIC_REG_TIMER_CURRCNT = 0x390,
    LAPIC_REG_TIMER_DIV = 0x3e0,
};

enum apic_timer_division
{
    APIC_TIMER_DIVIDE_BY_2 = 0,
    APIC_TIMER_DIVIDE_BY_4 = 1,
    APIC_TIMER_DIVIDE_BY_8 = 2,
    APIC_TIMER_DIVIDE_BY_16 = 3,
    APIC_TIMER_DIVIDE_BY_32 = 4,
    APIC_TIMER_DIVIDE_BY_64 = 5,
    APIC_TIMER_DIVIDE_BY_128 = 6,
    APIC_TIMER_DIVIDE_BY_1 = 7
};

typedef struct [[gnu::packed]]
{
    SdtHeader header;
    uint32_t local_controller_address;
    uint32_t flags;
    uint8_t entries[];
} Madt;

typedef struct [[gnu::packed]]
{
    uint8_t type;
    uint8_t length;
} MadtEntry;

typedef struct [[gnu::packed]]
{
    MadtEntry header;
    uint8_t ioapic_id;
    uint8_t _reserved;
    uint32_t ioapic_addr;
    uint32_t gsib;
} MadtIoapic;

typedef struct [[gnu::packed]]
{
    uint8_t version;
    uint8_t reserved;
    uint8_t max_redirect;
    uint8_t reserved2;
} IoapicVer;

typedef union [[gnu::packed]]
{
    struct [[gnu::packed]]
    {
        uint8_t vector;
        uint8_t delivery_mode : 3;
        uint8_t dest_mode : 1;
        uint8_t delivery_status : 1;
        uint8_t polarity : 1;
        uint8_t remote_irr : 1;
        uint8_t trigger : 1;
        uint8_t mask : 1;
        uint8_t reserved : 7;
        uint8_t dest_id;
    };

    struct [[gnu::packed]]
    {
        uint32_t low_byte;
        uint32_t high_byte;
    } _raw;
} IoapicRedirect;

typedef struct [[gnu::packed]]
{
    MadtEntry header;
    uint8_t bus_src;
    uint8_t irq_src;
    uint32_t gsi;
    uint16_t flags;
} MadtIso;

Res apic_init(void);
void lapic_eoi(void);
int lapic_id(void);
void ioapic_redirect_irq(uint32_t lapic_id, uint8_t intno, uint8_t irq);
void lapic_timer_start(void);
void lapic_timer_stop(void);