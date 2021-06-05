//this file is only ever included by default when compiling for ARM, or by direct inclusionwhich will cause issues

namespace CPUInfoShifts {
    //x86 extensions exist to prevent the need of preprocessors in your code, they will not return sensible values though, please either use shifts0::HELIUM or shifts0::NEON as these values work
    enum shifts0 {
        _3DNOW, _3DNOWEXT, ABM, ADX, AES, AVX, AVX2, AVX512CD, AVX512ER, AVX512F, AVX512PF, BMI1, BMI2, CLFSH, CMPXCHG16B, CX8, ERMS, F16C, FMA, FSGSBASE, FXSR, HLE, INVPCID, LAHF, LZCNT, MMX, MMXEXT, MONITOR, MOVBE, MSR, OSXSAVE, PCLMULQDQ, HELIUM = 0, NEON = 1
    };
    //x86 extensions exist to prevent the need of preprocessors in your code, they will not return sensible values though, please either use shifts0::HELIUM or shifts0::NEON as these values work
    enum shifts1 {
        POPCNT, PREFETCHWT1, RDRAND, RDSEED, RDTSCP, RTM, SEP, SHA, SSE, SSE2, SSE3, SSE4_1, SSE4_2, SSE4a, SSSE3, SYSCALL, TBM, XOP, XSAVE
    };
}