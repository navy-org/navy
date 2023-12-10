#pragma once

#include <stdint.h>

#define IDT_ENTRY_COUNT (256)

enum
{
    IDT_INT_PRESENT = (1 << 7),
    IDT_INT_GATE = (0xe),
};

typedef struct [[gnu::packed]]
{
    uint16_t size;
    uintptr_t offset;
} IdtDescriptor;

typedef struct packed
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} IdtEntry;

typedef struct [[gnu::packed]]
{
    IdtEntry entries[IDT_ENTRY_COUNT];
} Idt;

void idt_init(void);

uintptr_t idt_descriptor(void);

extern uintptr_t __interrupts_vector[];

extern void idt_flush(uintptr_t);