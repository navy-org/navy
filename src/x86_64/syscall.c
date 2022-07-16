#include "syscall.h"
#include <copland/debug.h>

#include <unistd.h>

void syscall_init(void)
{
    asm_write_msr(MSR_EFER, asm_read_msr(MSR_EFER) | 1);
    asm_write_msr(MSR_STAR, ((uint64_t)(GDT_KERNEL_CODE * 8) << STAR_KCODE_OFFSET) | ((uint64_t) (((GDT_USER_DATA - 1) * 8) | 3 ) << STAR_UCODE_OFFSET));
    asm_write_msr(MSR_LSTAR, (uint64_t) syscall_handle);
    asm_write_msr(MSR_SYSCALL_FLAG_MASK, 0xfffffffe);
}

int64_t syscall_debug_log(Regs *reg)
{
    Writer *out = get_debug_out();
    out->puts(out, (char const *) reg->rbx);
    return 0;
}

Syscall syscall_matrix[] = {
    [SYS_DEBUG] = syscall_debug_log
};

int64_t syscall_handler(Regs *regs)
{
    log$("SYSCALL !!!");
    return syscall_matrix[regs->rax](regs);
}