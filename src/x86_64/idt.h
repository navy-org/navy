#pragma once 

#include <copland/macro.h>
#include <stdint.h>

#define IDT_LENGTH 256

typedef struct PACKED 
{
    uint16_t offset_1;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_2;
    uint32_t offset_3;
    uint32_t zero;
} IdtDecriptor;

typedef enum 
{
    INTGATE = 0x8e,
    TRAPGATE = 0xef,
} InterruptType;

typedef struct PACKED
{
    uint16_t limit;
    uintptr_t base;
} IdtPointer;

void idt_init(void);

extern uintptr_t __interrupt_vector[];
extern void idt_flush(uintptr_t ptr);