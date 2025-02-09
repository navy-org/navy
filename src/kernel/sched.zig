const std = @import("std");
const arch = @import("arch");
const Task = @import("./task.zig").Task;
const Spinlock = @import("sync").Spinlock;
const log = std.log.scoped(.sched);

pub var lock = Spinlock.init();

const QUANTUM = 64;

const Internals = struct {
    const TaskList = std.DoublyLinkedList(*Task);
    var tasks = TaskList{};

    var current_task: *TaskList.Node = undefined;
    var ticks: usize = 0;
    var pid: u32 = 0;

    var gpa = std.heap.GeneralPurposeAllocator(.{ .MutexType = Spinlock }){};
    const alloc = gpa.allocator();
};

pub fn push_task(task: *Task) !void {
    task.pid = Internals.pid;
    Internals.pid += 1;
    var node = try Internals.alloc.create(Internals.TaskList.Node);
    node.data = task;
    Internals.tasks.append(node);
}

pub fn setup() !void {
    const kernelSpace = arch.paging.kernelSpace;
    try push_task(try Task.new("kernel", 0, kernelSpace(), false));
    Internals.current_task = Internals.tasks.first.?;
}

pub fn current() *Task {
    return Internals.current_task.data;
}

pub fn get_task(pid: u32) *Task {
    var task = Internals.tasks.first.?;
    while (task) |t| : (task = t.next) {
        if (t.data.pid == pid) {
            return t.data;
        }
    }
}

pub fn yield(regs: *arch.context.Registers) void {
    lock.lock();
    defer lock.unlock();

    arch.as.disableInterrupts();
    defer arch.as.enableInterrupts();

    if (Internals.tasks.len < 2) {
        return;
    }

    if (Internals.ticks < QUANTUM) {
        Internals.ticks += 1;
        return;
    }

    Internals.ticks = 0;
    Internals.current_task.data.ctx.save(regs);

    var task: *Internals.TaskList.Node = undefined;

    if (Internals.current_task.next == null) {
        task = Internals.tasks.first.?;
    } else {
        task = Internals.current_task.next.?;
    }

    task.data.ctx.restore(regs);

    task.data.space.load();
    Internals.current_task = task;
}
