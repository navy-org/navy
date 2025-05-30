const std = @import("std");
const arch = @import("arch");
const capability = @import("./capability.zig");
const Spinlock = @import("sync").Spinlock;

const AnyCap = capability.AnyCap;
const CNode = capability.CNode;
const Task = @import("./task.zig").Task;
const Channel = @import("./channel.zig").Channel;
const Error = error{TaskEntryNotFound};

const log = std.log.scoped(.sched);

pub var lock = Spinlock.init();

const QUANTUM = 32;

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

    if (task.pid > 1) {
        var channel = try Channel.new();
        const cap = channel.capability();

        const bus = try get_task(1);
        const cnode: *CNode = @ptrCast(@alignCast(bus.caps.items[1].context));
        try cnode.caps.append(cap);

        try task.caps.append(cap);
    } else if (task.pid == 1) {
        var cnode = try CNode.new();
        try task.caps.append(cnode.capability());
    }

    Internals.pid += 1;
    var node = try Internals.alloc.create(Internals.TaskList.Node);
    node.data = task;
    Internals.tasks.append(node);
}

pub fn setup() !void {
    const kernelSpace = arch.paging.kernelSpace;
    try push_task(try Task.new("kernel", 0, kernelSpace(), false, &[_][]const u8{}));
    Internals.current_task = Internals.tasks.first.?;
}

pub fn current() ?*Task {
    if (Internals.pid == 0) {
        // NOTE: That means that the scheduler is most likely not initialised
        return null;
    }
    return Internals.current_task.data;
}

pub fn get_task(pid: u32) !*Task {
    var task = Internals.tasks.first;
    while (task) |t| : (task = t.next) {
        if (t.data.pid == pid) {
            return t.data;
        }
    }

    return Error.TaskEntryNotFound;
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
