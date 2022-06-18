#pragma once 

#include "acpi.h"

typedef struct 
{
    AcpiSdt header;
    uint32_t lapic_address;

    union 
    {
        uint32_t flags;
        struct 
        {
            uint16_t type;
            uint16_t lenght;
        };
    };
} Apic;

void apic_init(void);