#include <dbg/log.h>
#include <hal.h>
#include <handover.h>
#include <kmalloc/kmalloc.h>
#include <mem/stack.h>
#include <specs/elf.h>
#include <string.h>
#include <tinyvmem/tinyvmem.h>

#include "x86_64/ctx.h"

#include "hal.h"
#include "pmm.h"
#include "pre-sched.h"

#ifdef __ck_bits_64__
#    define Elf_Ehdr Elf64_Ehdr
#    define ELFCLASS ELFCLASS64
#    define Elf_Phdr Elf64_Phdr
#else
#    define Elf_Ehdr Elf32_Ehdr
#    define ELFCLASS ELFCLASS32
#    define Elf_Phdr Elf32_Phdr
#endif

static HalPage *sched_vspace;
static HalContext *sched_ctx;
static Vmem sched_vmem;
static Stack sched_stack;

static HalContext *kernel_ctx;
static bool need_switch = false;

Res load_scheduler(void)
{
    HandoverRecord sched = handover_file_find(handover(), "/bin/procman");
    if (sched.size == 0)
    {
        return err$(RES_NOENT);
    }

    try$(hal_space_create(&sched_vspace));

    vmem_init(&sched_vmem, "procman", (void *)USER_HEAP_BASE, USER_HEAP_SIZE, PMM_PAGE_SIZE, 0, 0, 0, 0, 0);

    Elf_Ehdr *hdr = (Elf_Ehdr *)sched.start;
    if (memcmp(hdr->e_ident, ELFMAG, SELFMAG) != 0)
    {
        return err$(RES_INVAL);
    }

    if (hdr->e_ident[EI_CLASS] != ELFCLASS)
    {
        return err$(RES_INVAL);
    }

    for (size_t i = 0; i < hdr->e_phnum; i++)
    {
        Elf_Phdr *phdr = (Elf_Phdr *)(sched.start + hdr->e_phoff + i * hdr->e_phentsize);

        if (phdr->p_type == PT_LOAD)
        {
            log$("Mapping scheduler segment 0x%x -> 0x%x (len: %x)",
                 phdr->p_vaddr, phdr->p_vaddr + phdr->p_memsz, phdr->p_memsz);

            PmmObj paddr = pmm_alloc(align_up$(phdr->p_memsz, PMM_PAGE_SIZE));
            if (paddr.len == 0)
            {
                return err$(RES_NOMEM);
            }

            log$("Scheduler segment mapped to 0x%x", paddr.base);
            try$(hal_space_map(sched_vspace, phdr->p_vaddr, paddr.base, align_up$(phdr->p_memsz, PMM_PAGE_SIZE), HAL_MEM_READ | HAL_MEM_WRITE | HAL_MEM_USER | HAL_MEM_EXEC));

            memcpy((void *)hal_mmap_l2h(paddr.base), (void *)sched.start + phdr->p_offset, phdr->p_filesz);
            memset((void *)hal_mmap_l2h(paddr.base + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
        }
    }

    sched_ctx = (HalContext *)try$(hal_context_create());
    kernel_ctx = (HalContext *)try$(hal_context_create());

    PmmObj sched_stack_obj = pmm_alloc(align_up$(USER_STACK_SIZE, PMM_PAGE_SIZE) / PMM_PAGE_SIZE);
    if (sched_stack_obj.len == 0)
    {
        return err$(RES_NOMEM);
    }

    HandoverBuilder builder;
    PmmObj handover_obj = pmm_alloc(align_up$(kib$(16), PMM_PAGE_SIZE) / PMM_PAGE_SIZE);
    if (handover_obj.len == 0)
    {
        return err$(RES_NOMEM);
    }

    handover_builder_init(&builder, (void *)hal_mmap_l2h(handover_obj.base), kib$(16));
    handover_parse_module(&builder);

    builder.payload->magic = HANDOVER_MAGIC;

    uintptr_t handover_addr = (uintptr_t)vmem_alloc(&sched_vmem, kib$(16), VM_INSTANTFIT);

    Alloc kmalloc = kmalloc_acquire();
    sched_stack = stack_init(hal_mmap_l2h(sched_stack_obj.base), USER_STACK_SIZE);
    uintptr_t *argc = (uintptr_t *)try$(kmalloc.calloc(1, sizeof(uintptr_t)));
    *argc = 2;

    uintptr_t *argv = (uintptr_t *)try$(kmalloc.calloc(2, sizeof(uintptr_t)));
    argv[0] = USER_STACK_TOP - stack_push(&sched_stack, "boot:///bin/procman\0", 20);
    argv[1] = handover_addr;

    stack_push(&sched_stack, argv, 2 * sizeof(uintptr_t));
    stack_push(&sched_stack, (void *)argc, sizeof(uintptr_t));

    kmalloc.free(argc);
    kmalloc.free(argv);

    try$(hal_space_map(sched_vspace, align_down$(handover_addr, PMM_PAGE_SIZE), align_down$((uintptr_t)handover_obj.base, PMM_PAGE_SIZE),
                       align_up$(builder.size, PMM_PAGE_SIZE), HAL_MEM_READ | HAL_MEM_USER));

    try$(hal_space_map(sched_vspace, USER_STACK_BASE, sched_stack_obj.base, USER_STACK_SIZE, HAL_MEM_READ | HAL_MEM_WRITE | HAL_MEM_USER));

    hal_context_start(sched_ctx, hdr->e_entry, USER_STACK_TOP - sched_stack.off, (SysArgs){0, 0, 0, 0, 0, 0});
    need_switch = true;

    return ok$();
}

void switch_to_scheduler(HalRegs *regs)
{
    if (need_switch)
    {
        hal_context_save(kernel_ctx, regs);

        hal_space_apply((HalPage *)sched_vspace);
        hal_context_restore(sched_ctx, regs);

        need_switch = false;

        return;
    }
}