void hal_disable_interrupts(void)
{
    asm volatile("cli");
}

void hal_enable_interrupts(void)
{
    asm volatile("sti");
}

void hal_pause(void)
{
    asm volatile("pause");
}

void hal_panic(void)
{
    asm volatile("int $1");
}
