const logger = @import("logger");

const CPUID_EXTENDED_LEAF: u32 = 0x80000001;
const CPUID_EXFEATURE_PDPE1GB: u32 = 1 << 26;
const CPUID_XSAVE_SUPPORT: u32 = 1 << 26;
const CPUID_AVX_SUPPORT: u32 = 1 << 28;
const CPUID_AVX512_SUPPORT: u32 = 1 << 16;
const CPUID_FEATURE_IDENTIFIER: u32 = 0x1;
const CPUID_EXTENDED_FEATURE_IDENTIFIER: u32 = 0x7;
const CPUID_PROC_EXTENDED_STATE_ENUMERATION: u32 = 0xD;

const CpuidResult = struct {
    eax: u32,
    ebx: u32,
    ecx: u32,
    edx: u32,
};

const CpuidError = error{CpuidNotSupported};

pub fn cpuid(leaf: u32, subleaf: u32) !CpuidResult {
    const cpuid_max: u32 = asm volatile (
        \\ cpuid
        : [ret] "={eax}" (-> u32),
        : [_] "{eax}" (leaf & 0x80000000),
          [_] "{ebx}" (0),
          [_] "{ecx}" (0),
          [_] "{edx}" (0),
    );

    if (cpuid_max < leaf) {
        return error.CpuidNotSupported;
    }

    var eax: u32 = 0;
    var ebx: u32 = 0;
    var ecx: u32 = 0;
    var edx: u32 = 0;

    asm volatile (
        \\ cpuid
        : [_] "={eax}" (eax),
          [_] "={ebx}" (ebx),
          [_] "={ecx}" (ecx),
          [_] "={edx}" (edx),
        : [_] "{eax}" (leaf),
          [_] "{ecx}" (subleaf),
    );

    return .{
        .eax = eax,
        .ebx = ebx,
        .ecx = ecx,
        .edx = edx,
    };
}

fn lazyCpuid(comptime leaf: u32, comptime subleaf: u32, comptime reg: []const u8, comptime cmp: u32) fn () CpuidError!bool {
    return struct {
        fn wrap() !bool {
            const state = struct {
                var result: ?bool = null;
            };

            if (state.result == null) {
                const cpuid_result = try cpuid(leaf, subleaf);
                state.result = (@field(cpuid_result, reg) & cmp) != 0;
            }

            return state.result.?;
        }
    }.wrap;
}

pub const has1GBPages = lazyCpuid(CPUID_EXTENDED_LEAF, 0, "ecx", CPUID_EXFEATURE_PDPE1GB);
