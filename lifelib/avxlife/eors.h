#pragma once
#include <stdint.h>
#include <cstring>
#include <cpuid.h>

/*
* This calls the CPUID instruction to determine the capabilities of the
* underlying architecture.
*/

namespace apg {

    char __cpu_name[] = "I don't know";
    uint32_t __best_instruction_set = 0;

    // Linear progression of increasingly good instruction sets:
    uint32_t __have_mmx = 0;     // 2
    uint32_t __have_sse = 0;     // 3
    uint32_t __have_sse2 = 0;    // 4
    uint32_t __have_sse3 = 0;    // 5
    uint32_t __have_ssse3 = 0;   // 6
    uint32_t __have_sse4_1 = 0;  // 7
    uint32_t __have_sse4_2 = 0;  // 8
    uint32_t __have_avx = 0;     // 9
    uint32_t __have_avx2 = 0;    // 10
    uint32_t __have_avx512 = 0;  // 11

    // AVX-512 subsets:
    uint32_t __have_avx512f = 0;
    uint32_t __have_avx512dq = 0;
    uint32_t __have_avx512ifma = 0;
    uint32_t __have_avx512pf = 0;
    uint32_t __have_avx512er = 0;
    uint32_t __have_avx512cd = 0;
    uint32_t __have_avx512bw = 0;
    uint32_t __have_avx512vl = 0;

    // Miscellaneous bonus instructions:
    uint32_t __have_aes = 0;
    uint32_t __have_sha = 0;
    uint32_t __have_popcnt = 0;
    uint32_t __have_bmi1 = 0;
    uint32_t __have_bmi2 = 0;
    uint32_t __have_fma = 0;
    uint32_t __have_fma3 = 0;

    /*
    * Apple Bottom's vector instruction set detector (modified)
    */
    int best_instruction_set() {

        if (__best_instruction_set == 0) {

            uint32_t eax, ebx, ecx, edx;

            __cpuid(0, eax, ebx, ecx, edx);
            uint32_t max_level = eax;
            std::memcpy(__cpu_name, &ebx, 4);
            std::memcpy(__cpu_name + 4, &edx, 4);
            std::memcpy(__cpu_name + 8, &ecx, 4);
            __cpuid(1, eax, ebx, ecx, edx);

            __have_mmx = ((edx >> 23) & 1);
            __have_sse = ((edx >> 25) & 1);
            __have_sse2 = ((edx >> 26) & 1);

            __have_sse3 = (ecx & 1);
            __have_ssse3 = ((ecx >> 9) & 1);
            __have_fma = ((ecx >> 12) & 1);
            __have_fma3 = ((ecx >> 12) & 1);
            __have_sse4_1 = ((ecx >> 19) & 1);
            __have_sse4_2 = ((ecx >> 20) & 1);
            __have_popcnt = ((ecx >> 23) & 1);
            __have_aes = ((ecx >> 25) & 1);
            __have_avx = ((ecx >> 28) & 1);

            if (max_level >= 7) {
                __cpuid_count(7, 0, eax, ebx, ecx, edx);
                __have_bmi1 = ((ebx >> 3) & 1);
                __have_avx2 = ((ebx >> 5) & 1);
                __have_bmi2 = ((ebx >> 8) & 1);
                __have_avx512 = ((ebx >> 16) & 1);
                __have_avx512f = ((ebx >> 16) & 1);
                __have_avx512dq = ((ebx >> 17) & 1);
                __have_avx512ifma = ((ebx >> 21) & 1);
                __have_avx512pf = ((ebx >> 26) & 1);
                __have_avx512er = ((ebx >> 27) & 1);
                __have_avx512cd = ((ebx >> 28) & 1);
                __have_sha = ((ebx >> 29) & 1);
                __have_avx512bw = ((ebx >> 30) & 1);
                __have_avx512vl = ((ebx >> 31) & 1);
            }

            if (__have_avx512) { __best_instruction_set = 11; } else
            if (__have_avx2) { __best_instruction_set = 10; } else
            if (__have_avx) { __best_instruction_set = 9; } else
            if (__have_sse4_2) { __best_instruction_set = 8; } else
            if (__have_sse4_1) { __best_instruction_set = 7; } else
            if (__have_ssse3) { __best_instruction_set = 6; } else
            if (__have_sse3) { __best_instruction_set = 5; } else
            if (__have_sse2) { __best_instruction_set = 4; } else
            if (__have_sse) { __best_instruction_set = 3; } else
            if (__have_mmx) { __best_instruction_set = 2; } else
            if (true) { __best_instruction_set = 1; }
        }

        return __best_instruction_set;
    }

    char* cpu_name() {

        best_instruction_set();
        return __cpu_name;

    }
}
