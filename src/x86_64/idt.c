#include "idt.h"
#include "asm.h"
#include "gdt.h"

#include <stddef.h>
#include <copland/debug.h>

static IdtDecriptor idt[IDT_LENGTH] = {0};    
static IdtPointer idtDescriptor = {
    .base = (uintptr_t) &idt[0],
    .limit = sizeof(idt) - 1
};


void idt_descr_init(uintptr_t offset, InterruptType type, IdtDecriptor *self)
{
    self->offset_1 = offset;
    self->offset_2 = (offset >> 16);
    self->offset_3 = (offset >> 32);

    self->ist = 0;
    self->selector = GDT_KERNEL_CODE * 8;
    self->type_attr = type;
    self->zero = 0;
}

void idt_init(void)
{
    for (size_t i = 0; i < 49; i++)
    {
        idt_descr_init(__interrupt_vector[i], INTGATE, &idt[i]);
    }

    idt_flush((uintptr_t) &idtDescriptor);
}