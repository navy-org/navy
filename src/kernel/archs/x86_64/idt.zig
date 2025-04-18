const logger = @import("logger");
const std = @import("std");
const elf = @import("elf");
const sched = @import("kernel").sched;
const pmm = @import("./pmm.zig");

const GdtType = @import("./gdt.zig").GdtType;
const as = @import("./asm.zig");
const Registers = @import("./regs.zig").Registers;
const limine = @import("./limine.zig");
const PageAllocator = pmm.PageAllocator;
const Lapic = @import("./acpi/apic.zig").Lapic;
const Serial = @import("./serial.zig").Serial;
const Spinlock = @import("sync").Spinlock;

extern fn idt_flush(addr: u64) void;
extern const __interrupts_vector: [IDT_ENTRY_COUNT]u64;

const log = std.log.scoped(.idt);
const IDT_ENTRY_COUNT: usize = 256;
const IDT_INTERRUPT_PRESENT: usize = (1 << 7);
const IDT_INTERRUPT_GATE: usize = 0xe;
const INT_BREAKPOINT = 3;
const INT_PAGEFAULT = 0xe;

var idt: Idt = std.mem.zeroes(Idt);

const IdtEntry = packed struct(u128) {
    offset_low: u16,
    selector: u16,
    ist: u3,
    reserved0: u5 = 0,
    gate_type: u4,
    zero: u1,
    dpl: u2,
    present: u1,
    offset_middle: u16,
    offset_high: u32,
    reserved1: u32 = 0,

    pub fn init(base: u64, gate_type: u4) IdtEntry {
        return .{
            .ist = 0,
            .zero = 0,
            .present = 1,
            .offset_low = @intCast(base & 0xffff),
            .offset_middle = @intCast((base >> 16) & 0xffff),
            .offset_high = @intCast(base >> 32 & 0xffffffff),
            .selector = @as(u16, @intFromEnum(GdtType.KernelCode)) * 8,
            .gate_type = gate_type,
            .dpl = 0,
        };
    }
};

const Idt = extern struct {
    entries: [IDT_ENTRY_COUNT]IdtEntry,
};

const IdtDescriptor = packed struct {
    size: u16,
    offset: u64,

    pub fn load(_idt: *const Idt) IdtDescriptor {
        return .{
            .size = @intCast(@sizeOf(Idt) - 1),
            .offset = @intFromPtr(_idt),
        };
    }

    pub fn apply(self: *const IdtDescriptor) void {
        idt_flush(@intFromPtr(self));
    }
};

pub fn setup() void {
    var i: usize = 0;
    for (__interrupts_vector) |base| {
        idt.entries[i] = IdtEntry.init(base, IDT_INTERRUPT_GATE);
        i += 1;
    }

    IdtDescriptor.load(&idt).apply();
    log.debug("IDT loaded", .{});
}

// === Interrupt handlers =====================================================

const exception_message: [32][]const u8 = .{
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Detected Overflow",
    "Out Of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad Tss",
    "Segment Not Present",
    "StackFault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Hypervisor Injection Exception",
    "paging Communication Exception",
    "Security Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
};

var sym: ?elf.Symbols = null;
var lock = Spinlock.init();

fn dumpRegs(regs: *Registers) void {
    lock.lock();
    defer lock.unlock();

    var backtrace = std.debug.StackIterator.init(@returnAddress(), null);
    defer backtrace.deinit();

    if (sym == null) {
        // if (limine.kernel.response) |k| {
        //     var pallocator = PageAllocator.new();
        //     const alloc = pallocator.allocator();
        //
        //     const bin = elf.Elf.fromSlice(k.kernel_file.address);
        //     sym = elf.Symbols.from_elf(bin, .little, alloc) catch null;
        // }
    }

    logger.print("\n!!! ---------------------------------------------------------------------------------------------------\n\n", .{});
    if (regs.intno != INT_BREAKPOINT) {
        logger.print("    KERNEL PANIC\n\n", .{});
        logger.print("    {s} was raised\n", .{exception_message[regs.*.intno]});
        logger.print("    interrupt: {x}, err: {x}\n\n", .{ regs.*.intno, regs.*.err });
    } else {
        logger.print("    BREAKPOINT\n\n", .{});
    }

    if (sched.current()) |task| {
        logger.print("    Current task: {s}\n", .{task.name});
    }

    logger.print("    RAX {x:0>16} RBX {x:0>16} RCX {x:0>16} RDX {x:0>16}\n", .{ regs.*.rax, regs.*.rbx, regs.*.rcx, regs.*.rdx });
    logger.print("    RSI {x:0>16} RDI {x:0>16} RBP {x:0>16} RSP {x:0>16}\n", .{ regs.*.rsi, regs.*.rdi, regs.*.rbp, regs.*.rsp });
    logger.print("    R8  {x:0>16} R9  {x:0>16} R10 {x:0>16} R11 {x:0>16}\n", .{ regs.*.r8, regs.*.r9, regs.*.r10, regs.*.r11 });
    logger.print("    R12 {x:0>16} R13 {x:0>16} R14 {x:0>16} R15 {x:0>16}\n", .{ regs.*.r12, regs.*.r13, regs.*.r14, regs.*.r15 });
    logger.print("    CR0 {x:0>16} CR2 {x:0>16} CR3 {x:0>16} CR4 {x:0>16}\n", .{ as.cr0.read(), as.cr2.read(), as.cr3.read(), as.cr4.read() });
    logger.print("    CS  {x:0>16} SS  {x:0>16} FLG {x:0>16}\n", .{ regs.*.cs, regs.*.ss, regs.*.rflags });
    logger.print("    RIP \x1B[7m{x:0>16}\x1B[0m ", .{regs.*.rip});

    if (sym) |*s| {
        const info = s.infoForAddr(regs.rip) catch |err| {
            log.err("Error: {any}\n", .{err});
            return;
        };

        logger.print("\n\n    Exception occurred at\n    {s}:{d} ({s})\n", .{ info.lineInfo.file_name, info.lineInfo.line, info.symbol });
    }

    logger.print("\n    Backtrace:\n\n", .{});

    while (backtrace.next()) |address| {
        if (address == 0) {
            break;
        }

        if (sym) |*s| {
            const info = s.infoForAddr(address) catch {
                logger.print("    ?* 0x{x:0>16}\n", .{address});
                continue;
            };

            logger.print("    * {s: <70}: {d: <3} ({s})\n", .{ info.lineInfo.file_name, info.lineInfo.line, info.symbol });
        } else {
            logger.print("    * 0x{x:0>16}\n", .{address});
        }
    }

    logger.print("\n--------------------------------------------------------------------------------------------------- !!!\n\n", .{});
}

pub export fn interrupt_handler(rsp: u64) callconv(.C) u64 {
    const regs = Registers.fromRsp(rsp);

    if (regs.intno == INT_PAGEFAULT) {
        if (sched.current()) |task| {
            if (task.pid > 0) {
                const cr2_aligned = std.mem.alignBackward(usize, as.cr2.read(), std.heap.pageSize());

                if (task.mm.mmap_lookup(cr2_aligned)) |area| {
                    task.mm.mmap_remove(cr2_aligned);

                    var pallocator = PageAllocator.new();
                    const alloc = pallocator.allocator();

                    const slice = alloc.alloc(u8, std.heap.pageSize()) catch |e| {
                        log.err("Couldn't map allocated memory: {any}", .{e});
                        as.hlt();
                    };

                    task.space.map(cr2_aligned, pmm.upper2lower(@intFromPtr(slice.ptr)), slice.len, area.flags) catch |e| {
                        log.err("Couldn't map allocated memory: {any}", .{e});
                        as.hlt();
                    };

                    Lapic.eoi();
                    return rsp;
                }
            }
        }
    }

    if (regs.intno == INT_BREAKPOINT) {
        dumpRegs(regs);

        var buffer: [8]u8 = std.mem.zeroes([8]u8);
        var allocator = std.heap.FixedBufferAllocator.init(&buffer);
        const alloc = allocator.allocator();

        logger.print("\n\nPress any key to continue...\n", .{});

        const s = Serial{};
        _ = s.read(alloc, 1) catch |err| {
            log.err("Failed to read from serial port: {any}\n", .{err});
            @panic("Failed to read from serial port");
        };

        Lapic.eoi();
        return rsp;
    } else if (regs.intno < exception_message.len) {
        dumpRegs(regs);

        if (sym) |*s| {
            s.deinit();
        }

        as.hlt();
    } else {
        const irq = regs.intno - exception_message.len;
        if (irq == 0) {
            sched.yield(regs);
        } else {}
    }

    Lapic.eoi();
    return rsp;
}
