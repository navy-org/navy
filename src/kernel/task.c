#include "task.h"
#include "const.h"
#include "pmm.h"
#include "x86_64/vmm.h"

#include <stdlib.h>

Task *task_create1(Str path)
{
    return task_create2(path, UNWRAP_OR_PANIC(vmm_create_space(), "Couldn't create address space"));
}

Task *task_create2(Str path, Pml *space)
{
    Task *self = (Task *) calloc(sizeof(Task), 1);
    self->state = TASK_READY;
    self->path = path;

    self->stack = UNWRAP_OR_PANIC(pmm_alloc(STACK_SIZE / PAGE_SIZE), "PMM couldn't allocate memory");
    self->space = space;

    vmm_map_range(self->space, (Range) {
        .base = USER_STACK_BASE,
        .length = STACK_SIZE / PAGE_SIZE
    }, self->stack, true);

    return self;
}
