#include "task.h"
#include "pmm.h"

#include <stdlib.h>

Task *task_create_impl(Str path, Pml *space)
{
    Task *self = (Task *) calloc(sizeof(Task), 1);
    self->state = TASK_READY;
    self->path = path;

    self->stack = UNWRAP_OR_PANIC(pmm_alloc(STACK_SIZE / PAGE_SIZE), "PMM couldn't allocate memory");
    self->space = space == NULL ? UNWRAP_OR_PANIC(vmm_create_space(), "Couldn't create address space") : space;

    vmm_map_range(self->space, (Range) {
        .base = USER_STACK_BASE,
        .length = self->stack.length, 
    }, self->stack, true);

    return self;
}
