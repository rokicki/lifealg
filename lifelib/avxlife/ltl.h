#pragma once
#include "eors.h"

namespace apg {

    const static uint8_t __transperm[] __attribute__((aligned(64))) = {0,
        4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15};

    // Actually 32 ones, to make it easier for avx2:
    const static uint8_t __sixteen1s[] __attribute__((aligned(64))) = {1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

    void bits2bytes_sse2(uint64_t *inleafx, uint64_t *quarter) {

        asm (
        // Load data from memory into registers:
        "movups (%0), %%xmm0 \n\t"
        "movups 16(%0), %%xmm1 \n\t"
        "movups (%2), %%xmm3 \n\t"
        // Transpose this 2x2 matrix of uint64s:
        "movdqa %%xmm0, %%xmm2 \n\t"
        "shufps $0x44, %%xmm1, %%xmm0 \n\t"
        "shufps $0xee, %%xmm1, %%xmm2 \n\t"
        // Split bytes into bits and save to memory:
        "movdqa %%xmm0, %%xmm1 \n\t"
        "pand %%xmm3, %%xmm1 \n\t"
        "movups %%xmm1, (%1) \n\t"
        "psrld $1, %%xmm0 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "pand %%xmm3, %%xmm1 \n\t"
        "movups %%xmm1, 16(%1) \n\t"
        "psrld $1, %%xmm0 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "pand %%xmm3, %%xmm1 \n\t"
        "movups %%xmm1, 32(%1) \n\t"
        "psrld $1, %%xmm0 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "pand %%xmm3, %%xmm1 \n\t"
        "movups %%xmm1, 48(%1) \n\t"
        "psrld $1, %%xmm0 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "pand %%xmm3, %%xmm1 \n\t"
        "movups %%xmm1, 64(%1) \n\t"
        "psrld $1, %%xmm0 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "pand %%xmm3, %%xmm1 \n\t"
        "movups %%xmm1, 80(%1) \n\t"
        "psrld $1, %%xmm0 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "pand %%xmm3, %%xmm1 \n\t"
        "movups %%xmm1, 96(%1) \n\t"
        "psrld $1, %%xmm0 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "pand %%xmm3, %%xmm1 \n\t"
        "movups %%xmm1, 112(%1) \n\t"
        "movdqa %%xmm2, %%xmm0 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "pand %%xmm3, %%xmm1 \n\t"
        "movups %%xmm1, 128(%1) \n\t"
        "psrld $1, %%xmm0 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "pand %%xmm3, %%xmm1 \n\t"
        "movups %%xmm1, 144(%1) \n\t"
        "psrld $1, %%xmm0 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "pand %%xmm3, %%xmm1 \n\t"
        "movups %%xmm1, 160(%1) \n\t"
        "psrld $1, %%xmm0 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "pand %%xmm3, %%xmm1 \n\t"
        "movups %%xmm1, 176(%1) \n\t"
        "psrld $1, %%xmm0 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "pand %%xmm3, %%xmm1 \n\t"
        "movups %%xmm1, 192(%1) \n\t"
        "psrld $1, %%xmm0 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "pand %%xmm3, %%xmm1 \n\t"
        "movups %%xmm1, 208(%1) \n\t"
        "psrld $1, %%xmm0 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "pand %%xmm3, %%xmm1 \n\t"
        "movups %%xmm1, 224(%1) \n\t"
        "psrld $1, %%xmm0 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "pand %%xmm3, %%xmm1 \n\t"
        "movups %%xmm1, 240(%1) \n\t"
        : /* no output operands -- implicitly volatile */
        : "r" (inleafx), "r" (quarter), "r" (__sixteen1s)
        : "xmm0", "xmm1", "xmm2", "xmm3", "memory" );

    }

    void bits2bytes(uint64_t *inleafx, uint64_t *quarter) {
        // Unpacks an inleaf into a 16-by-16 byte array, each element of
        // which is either 0 or 1:
        for (uint64_t x = 0; x < 2; x++) {
            for (uint64_t y = 0; y < 2; y++) {
                uint64_t l = inleafx[2*y+x];
                for (uint64_t j = 0; j < 8; j++) {
                    quarter[y+2*j+16*x] = (l >> j) & 0x0101010101010101ull;
                }
            }
        }
    }

    void bytes2bits(uint64_t *quarter, uint64_t *outleafx) {
        for (uint64_t x = 0; x < 2; x++) {
            for (uint64_t y = 0; y < 2; y++) {
                uint64_t l = 0;
                for (uint64_t j = 0; j < 8; j++) {
                    l |= ((quarter[y+2*j+16*x] & 0x0101010101010101ull) << j);
                }
                outleafx[2*y+x] = l;
            }
        }
    }

    void p6dwordcycle_sse2(uint64_t *a, uint64_t *b) {

        asm (
        "movups (%0), %%xmm0 \n\t"
        "movups 16(%0), %%xmm1 \n\t"
        "movups 32(%0), %%xmm2 \n\t"
        "movups 48(%0), %%xmm3 \n\t"
        "movdqa %%xmm0, %%xmm4 \n\t"
        "movdqa %%xmm2, %%xmm5 \n\t"
        "shufps $0x44, %%xmm1, %%xmm0 \n\t"
        "shufps $0xee, %%xmm1, %%xmm4 \n\t"
        "shufps $0x44, %%xmm3, %%xmm2 \n\t"
        "shufps $0xee, %%xmm3, %%xmm5 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "movdqa %%xmm4, %%xmm3 \n\t"
        "shufps $0x88, %%xmm2, %%xmm0 \n\t"
        "shufps $0xdd, %%xmm2, %%xmm1 \n\t"
        "shufps $0x88, %%xmm5, %%xmm3 \n\t"
        "shufps $0xdd, %%xmm5, %%xmm4 \n\t"
        "movups %%xmm0, (%1) \n\t"
        "movups %%xmm1, 64(%1) \n\t"
        "movups %%xmm3, 128(%1) \n\t"
        "movups %%xmm4, 192(%1) \n\t"
        : /* no output operands -- implicitly volatile */
        : "r" (a), "r" (b)
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory" );
        
    }

    void p6bytecycle_sse2(uint64_t *a, uint64_t *b) {

        asm (
        "movups (%0), %%xmm0 \n\t"
        "movups 16(%0), %%xmm1 \n\t"
        "movups 32(%0), %%xmm2 \n\t"
        "movups 48(%0), %%xmm3 \n\t"
        "movdqa %%xmm0, %%xmm4 \n\t"
        "movdqa %%xmm2, %%xmm5 \n\t"
        "punpcklbw %%xmm1, %%xmm0 \n\t"
        "punpckhbw %%xmm1, %%xmm4 \n\t"
        "punpcklbw %%xmm3, %%xmm2 \n\t"
        "punpckhbw %%xmm3, %%xmm5 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "movdqa %%xmm4, %%xmm3 \n\t"
        "punpcklwd %%xmm2, %%xmm0 \n\t"
        "punpckhwd %%xmm2, %%xmm1 \n\t"
        "punpcklwd %%xmm5, %%xmm3 \n\t"
        "punpckhwd %%xmm5, %%xmm4 \n\t"
        "movdqa %%xmm0, %%xmm2 \n\t"
        "movdqa %%xmm3, %%xmm5 \n\t"
        "punpcklbw %%xmm1, %%xmm0 \n\t"
        "punpckhbw %%xmm1, %%xmm2 \n\t"
        "punpcklbw %%xmm4, %%xmm3 \n\t"
        "punpckhbw %%xmm4, %%xmm5 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "movdqa %%xmm2, %%xmm4 \n\t"
        "punpcklwd %%xmm3, %%xmm0 \n\t"
        "punpckhwd %%xmm3, %%xmm1 \n\t"
        "punpcklwd %%xmm5, %%xmm4 \n\t"
        "punpckhwd %%xmm5, %%xmm2 \n\t"
        "movups %%xmm0, (%1) \n\t"
        "movups %%xmm1, 16(%1) \n\t"
        "movups %%xmm4, 32(%1) \n\t"
        "movups %%xmm2, 48(%1) \n\t"
        : /* no output operands -- implicitly volatile */
        : "r" (a), "r" (b)
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory" );

    }

    void p6dwordcycle_avx(uint64_t *a, uint64_t *b) {

        asm (
        "vmovups (%0), %%xmm0 \n\t"
        "vmovups 16(%0), %%xmm1 \n\t"
        "vmovups 32(%0), %%xmm2 \n\t"
        "vmovups 48(%0), %%xmm3 \n\t"
        "vshufps $0xee, %%xmm1, %%xmm0, %%xmm4 \n\t"
        "vshufps $0x44, %%xmm1, %%xmm0, %%xmm0 \n\t"
        "vshufps $0xee, %%xmm3, %%xmm2, %%xmm5 \n\t"
        "vshufps $0x44, %%xmm3, %%xmm2, %%xmm2 \n\t"
        "vshufps $0xdd, %%xmm2, %%xmm0, %%xmm1 \n\t"
        "vshufps $0x88, %%xmm2, %%xmm0, %%xmm0 \n\t"
        "vshufps $0x88, %%xmm5, %%xmm4, %%xmm3 \n\t"
        "vshufps $0xdd, %%xmm5, %%xmm4, %%xmm4 \n\t"
        "vmovups %%xmm0, (%1) \n\t"
        "vmovups %%xmm1, 64(%1) \n\t"
        "vmovups %%xmm3, 128(%1) \n\t"
        "vmovups %%xmm4, 192(%1) \n\t"
        : /* no output operands -- implicitly volatile */
        : "r" (a), "r" (b)
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory" );
        
    }

    void p6bytecycle_avx(uint64_t *a, uint64_t *b) {

        asm (
        "vmovups (%0), %%xmm0 \n\t"
        "vmovups 16(%0), %%xmm1 \n\t"
        "vmovups 32(%0), %%xmm2 \n\t"
        "vmovups 48(%0), %%xmm3 \n\t"
        "vpunpckhbw %%xmm1, %%xmm0, %%xmm4 \n\t"
        "vpunpcklbw %%xmm1, %%xmm0, %%xmm0 \n\t"
        "vpunpckhbw %%xmm3, %%xmm2, %%xmm5 \n\t"
        "vpunpcklbw %%xmm3, %%xmm2, %%xmm2 \n\t"
        "vpunpckhwd %%xmm2, %%xmm0, %%xmm1 \n\t"
        "vpunpcklwd %%xmm2, %%xmm0, %%xmm0 \n\t"
        "vpunpcklwd %%xmm5, %%xmm4, %%xmm3 \n\t"
        "vpunpckhwd %%xmm5, %%xmm4, %%xmm4 \n\t"
        "vpunpckhbw %%xmm1, %%xmm0, %%xmm2 \n\t"
        "vpunpcklbw %%xmm1, %%xmm0, %%xmm0 \n\t"
        "vpunpckhbw %%xmm4, %%xmm3, %%xmm5 \n\t"
        "vpunpcklbw %%xmm4, %%xmm3, %%xmm3 \n\t"
        "vpunpckhwd %%xmm3, %%xmm0, %%xmm1 \n\t"
        "vpunpcklwd %%xmm3, %%xmm0, %%xmm0 \n\t"
        "vpunpcklwd %%xmm5, %%xmm2, %%xmm4 \n\t"
        "vpunpckhwd %%xmm5, %%xmm2, %%xmm2 \n\t"
        "vmovups %%xmm0, (%1) \n\t"
        "vmovups %%xmm1, 16(%1) \n\t"
        "vmovups %%xmm4, 32(%1) \n\t"
        "vmovups %%xmm2, 48(%1) \n\t"
        : /* no output operands -- implicitly volatile */
        : "r" (a), "r" (b)
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory" );

    }

    void transpose_16x16(uint64_t *a, uint64_t *b, bool inplace) {

        uint64_t* c = (inplace ? a : b);
        int bis = best_instruction_set();

        if (bis >= 9) {
            p6dwordcycle_avx(a, b);
            p6dwordcycle_avx(a+8, b+2);
            p6dwordcycle_avx(a+16, b+4);
            p6dwordcycle_avx(a+24, b+6);
            p6bytecycle_avx(b, c);
            p6bytecycle_avx(b+8, c+8);
            p6bytecycle_avx(b+16, c+16);
            p6bytecycle_avx(b+24, c+24);
        } else {
            p6dwordcycle_sse2(a, b);
            p6dwordcycle_sse2(a+8, b+2);
            p6dwordcycle_sse2(a+16, b+4);
            p6dwordcycle_sse2(a+24, b+6);
            p6bytecycle_sse2(b, c);
            p6bytecycle_sse2(b+8, c+8);
            p6bytecycle_sse2(b+16, c+16);
            p6bytecycle_sse2(b+24, c+24);
        }

    }

    void inplace_cumsum_sse2(uint64_t *a) {
        asm (
#include "cumsum_sse2.asm"
        : /* no output operands -- implicitly volatile */
        : "r" (a) 
        : "xmm0", "xmm1", "memory" );
    }

    void parallel_diff_sse2(uint64_t *a, uint64_t *b, uint64_t *c) {
        asm (
#include "bytediff_sse2.asm"
        : /* no output operands -- implicitly volatile */
        : "r" (a) , "r" (b), "r" (c)
        : "xmm0", "xmm1", "memory" );
    }

    void inplace_cumsum_avx(uint64_t *a) {
        asm (
#include "cumsum_avx.asm"
        : /* no output operands -- implicitly volatile */
        : "r" (a) 
        : "xmm0", "xmm1", "memory" );
    }

    void parallel_diff_avx(uint64_t *a, uint64_t *b, uint64_t *c) {
        asm (
#include "bytediff_avx.asm"
        : /* no output operands -- implicitly volatile */
        : "r" (a) , "r" (b), "r" (c)
        : "xmm0", "xmm1", "memory" );
    }

    void hadd_avx(uint64_t *a, uint64_t *b, uint64_t *c, uint64_t *d) {
        asm (
#include "hadd_avx.asm"
        : /* no output operands -- implicitly volatile */
        : "r" (a) , "r" (b), "r" (c), "r" (d)
        : "xmm0", "xmm1", "memory" );
    }

    void hadd_avx2(uint64_t *a, uint64_t *b, uint64_t *c, uint64_t *d) {
        asm (
#include "hadd_avx2.asm"
        : /* no output operands -- implicitly volatile */
        : "r" (a) , "r" (b), "r" (c), "r" (d)
        : "xmm0", "xmm1", "memory" );
    }

    void hadd_sse2(uint64_t *a, uint64_t *b, uint64_t *c, uint64_t *d) {
        asm (
#include "hadd_sse2.asm"
        : /* no output operands -- implicitly volatile */
        : "r" (a) , "r" (b), "r" (c), "r" (d)
        : "xmm0", "xmm1", "memory" );
    }

    void ladd_avx(uint64_t *a, uint64_t *b, uint64_t *c, uint64_t *d) {
        asm (
#include "ladd_avx.asm"
        : /* no output operands -- implicitly volatile */
        : "r" (a) , "r" (b), "r" (c), "r" (d)
        : "xmm0", "xmm1", "memory" );
    }

    void ladd_avx2(uint64_t *a, uint64_t *b, uint64_t *c, uint64_t *d) {
        asm (
#include "ladd_avx2.asm"
        : /* no output operands -- implicitly volatile */
        : "r" (a) , "r" (b), "r" (c), "r" (d)
        : "xmm0", "xmm1", "memory" );
    }

    void ladd_sse2(uint64_t *a, uint64_t *b, uint64_t *c, uint64_t *d) {
        asm (
#include "ladd_sse2.asm"
        : /* no output operands -- implicitly volatile */
        : "r" (a) , "r" (b), "r" (c), "r" (d)
        : "xmm0", "xmm1", "memory" );
    }

    void hadd_var(uint64_t *a, uint64_t *b) {
        int bis = best_instruction_set();
        if (bis >= 10) {
            hadd_avx2(a + 16, a + 14, a + 18, b);
        } else if (bis >= 9) {
            hadd_avx(a + 16, a + 14, a + 18, b);
        } else {
            hadd_sse2(a + 16, a + 14, a + 18, b);
        }
        transpose_16x16(b, a, true);
    }

    void ladd_var(uint64_t *a, uint64_t *b) {
        int bis = best_instruction_set();
        if (bis >= 10) {
            ladd_avx2(a + 16, a + 14, a + 18, b);
        } else if (bis >= 9) {
            ladd_avx(a + 16, a + 14, a + 18, b);
        } else {
            ladd_sse2(a + 16, a + 14, a + 18, b);
        }
        transpose_16x16(b, a, true);
    }

    void destructive_convolve(uint64_t *a, uint64_t *b, int range) {
        // Destructively convolve a 32x16 array into a 16x16 array:
        int bis = best_instruction_set();
        if (bis >= 9) {
            inplace_cumsum_avx(a);
            parallel_diff_avx(a + 2 * (7 - range), a + 2 * (8 + range), b);
        } else {
            inplace_cumsum_sse2(a);
            parallel_diff_sse2(a + 2 * (7 - range), a + 2 * (8 + range), b);
        }
        transpose_16x16(b, a, true);
    }

    void ntsum2d(uint64_t *a, uint64_t *b) {
        uint64_t c[64];
        ladd_var(a, c);
        ladd_var(a+64, c+32);
        hadd_var(c, b);
    }

    void convolve2d(uint64_t *a, uint64_t *b, int range) {
        // Find the 16x16 centre of a 32x32 array after convolving
        // with a Moore neighbourhood of a given range. Obliterates
        // the original array.
        uint64_t c[64];
        destructive_convolve(a, c, range);
        destructive_convolve(a+64, c+32, range);
        destructive_convolve(c, b, range);
    }

    void cmpxor_sse2(uint64_t *a, uint64_t *b, uint64_t *c) {
        asm (
        "movups (%0), %%xmm0 \n\t"
        "movups (%1), %%xmm1 \n\t"
        "movups (%2), %%xmm2 \n\t"
        "pcmpgtb %%xmm2, %%xmm0 \n\t"
        "pcmpgtb %%xmm2, %%xmm1 \n\t"
        "pxor %%xmm0, %%xmm1 \n\t"
        "movups %%xmm1, (%2) \n\t"
        "movups 16(%0), %%xmm0 \n\t"
        "movups 16(%1), %%xmm1 \n\t"
        "movups 16(%2), %%xmm2 \n\t"
        "pcmpgtb %%xmm2, %%xmm0 \n\t"
        "pcmpgtb %%xmm2, %%xmm1 \n\t"
        "pxor %%xmm0, %%xmm1 \n\t"
        "movups %%xmm1, 16(%2) \n\t"
        "movups 32(%0), %%xmm0 \n\t"
        "movups 32(%1), %%xmm1 \n\t"
        "movups 32(%2), %%xmm2 \n\t"
        "pcmpgtb %%xmm2, %%xmm0 \n\t"
        "pcmpgtb %%xmm2, %%xmm1 \n\t"
        "pxor %%xmm0, %%xmm1 \n\t"
        "movups %%xmm1, 32(%2) \n\t"
        "movups 48(%0), %%xmm0 \n\t"
        "movups 48(%1), %%xmm1 \n\t"
        "movups 48(%2), %%xmm2 \n\t"
        "pcmpgtb %%xmm2, %%xmm0 \n\t"
        "pcmpgtb %%xmm2, %%xmm1 \n\t"
        "pxor %%xmm0, %%xmm1 \n\t"
        "movups %%xmm1, 48(%2) \n\t"
        : /* no output operands -- implicitly volatile */
        : "r" (a), "r" (b), "r" (c)
        : "xmm0", "xmm1", "xmm2", "memory" );
    }

    void ltl_kernel(uint64_t *inleaves, uint64_t *outleafx, int range, uint8_t bmin, uint8_t bmax, uint8_t smin, uint8_t smax) {

        uint64_t lb1 = (bmin ^ smin);
        uint64_t ub1 = ((bmax + 1) ^ (smax + 1));
        uint64_t lb0 = (bmin ^ 0x80ull) * 0x0101010101010101ull;
        uint64_t ub0 = ((bmax + 1) ^ 0x80ull) * 0x0101010101010101ull;

        uint64_t a[128];
        uint64_t b[32];
        uint64_t c[32];
        uint64_t lowerbounds[32];
        uint64_t upperbounds[32];
        uint64_t d[4] = {inleaves[3], inleaves[6], inleaves[9], inleaves[12]};
        bits2bytes_sse2(inleaves, a);
        bits2bytes_sse2(inleaves+4, (a+32));
        bits2bytes_sse2(inleaves+8, (a+64));
        bits2bytes_sse2(inleaves+12, (a+96));
        bits2bytes_sse2(d, c);
        convolve2d(a, b, range);
        for (uint64_t i = 0; i < 32; i++) {
            lowerbounds[i] = (c[i] * lb1) ^ lb0;
            upperbounds[i] = (c[i] * ub1) ^ ub0;
            b[i] ^= 0x8080808080808080ull;
        }
        cmpxor_sse2(lowerbounds, upperbounds, b);
        cmpxor_sse2(lowerbounds + 8, upperbounds + 8, b + 8);
        cmpxor_sse2(lowerbounds + 16, upperbounds + 16, b + 16);
        cmpxor_sse2(lowerbounds + 24, upperbounds + 24, b + 24);
        bytes2bits(b, outleafx);
    }

}
