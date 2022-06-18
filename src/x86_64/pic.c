#include "pic.h"

void pic_mask_interrupts(void)
{
    uint16_t port;
    uint8_t value;
    uint8_t irq;

    for (size_t i = 0; i < 16; i++)
    {
        if (i < 8) 
        {
            port = PIC1_DATA;
            irq = i;
        }
        else  
        {
            port = PIC2_DATA;
            irq = i - 8; 
        }

        value = asm_in8(port) | (1 << irq);
        asm_out8(port, value);
    }
}