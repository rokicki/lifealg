#pragma once
#include <stdint.h>

namespace apg {

    // The first 32 bytes are arguments to (V)PSHUFB; the remaining
    // 32 bytes are arguments to VPERMD. We align on a 64-byte boundary
    // for both (16-byte) SIMD necessity and to avoid cache misses.
    const static uint8_t __lifeperm[] __attribute__((aligned(64))) = {0,
        4, 8, 12, 2, 6, 10, 14, 1, 5, 9, 13, 3, 7, 11, 15,
        0, 4, 8, 12, 2, 6, 10, 14, 1, 5, 9, 13, 3, 7, 11, 15,
        0, 0, 0, 0, 4, 0, 0, 0, 2, 0, 0, 0, 6, 0, 0, 0,
        1, 0, 0, 0, 5, 0, 0, 0, 3, 0, 0, 0, 7, 0, 0, 0};

    const static uint8_t __linvperm[] __attribute__((aligned(64))) = {0,
        8, 4, 12, 1, 9, 5, 13, 2, 10, 6, 14, 3, 11, 7, 15,
        0, 8, 4, 12, 1, 9, 5, 13, 2, 10, 6, 14, 3, 11, 7, 15};

    void transpose_bytes_avx(uint64_t* a, uint64_t* b) {

        asm (
        "vmovups (%0), %%xmm0 \n\t"
        "vmovups 16(%0), %%xmm1 \n\t"
        "vmovups 32(%0), %%xmm2 \n\t"
        "vmovups 48(%0), %%xmm3 \n\t"
        // First two rounds of shuffling:
        "vpunpcklbw %%xmm1, %%xmm0, %%xmm4 \n\t"
        "vpunpckhbw %%xmm1, %%xmm0, %%xmm5 \n\t"
        "vpunpcklbw %%xmm5, %%xmm4, %%xmm0 \n\t"
        "vpunpckhbw %%xmm5, %%xmm4, %%xmm1 \n\t"
        "vpunpcklbw %%xmm3, %%xmm2, %%xmm4 \n\t"
        "vpunpckhbw %%xmm3, %%xmm2, %%xmm5 \n\t"
        "vpunpcklbw %%xmm5, %%xmm4, %%xmm2 \n\t"
        "vpunpckhbw %%xmm5, %%xmm4, %%xmm3 \n\t"
        // Perpendicular shuffle:
        "vpunpckhdq %%xmm2, %%xmm0, %%xmm4 \n\t"
        "vpunpckldq %%xmm2, %%xmm0, %%xmm0 \n\t"
        "vpunpckhdq %%xmm3, %%xmm1, %%xmm5 \n\t"
        "vpunpckldq %%xmm3, %%xmm1, %%xmm1 \n\t"
        // Save in transposed order:
        "vmovups %%xmm0, (%1) \n\t"
        "vmovups %%xmm4, 16(%1) \n\t"
        "vmovups %%xmm1, 32(%1) \n\t"
        "vmovups %%xmm5, 48(%1) \n\t"
        : /* no output operands -- implicitly volatile */
        : "r" (a), "r" (b)
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory" );

    }

    void transpose_bytes_sse2(uint64_t* a, uint64_t* b) {

        asm (
        "movups (%0), %%xmm0 \n\t"
        "movups 16(%0), %%xmm1 \n\t"
        "movups 32(%0), %%xmm2 \n\t"
        "movups 48(%0), %%xmm3 \n\t"
        // First two rounds of shuffling:
        "movdqa %%xmm0, %%xmm4 \n\t"
        "movdqa %%xmm2, %%xmm5 \n\t"
        "punpcklbw %%xmm1, %%xmm0 \n\t"
        "punpckhbw %%xmm1, %%xmm4 \n\t"
        "punpcklbw %%xmm3, %%xmm2 \n\t"
        "punpckhbw %%xmm3, %%xmm5 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "movdqa %%xmm2, %%xmm3 \n\t"
        "punpcklbw %%xmm4, %%xmm0 \n\t"
        "punpckhbw %%xmm4, %%xmm1 \n\t"
        "punpcklbw %%xmm5, %%xmm2 \n\t"
        "punpckhbw %%xmm5, %%xmm3 \n\t"
        // Perpendicular shuffle:
        "movdqa %%xmm0, %%xmm4 \n\t"
        "punpckldq %%xmm2, %%xmm0 \n\t"
        "punpckhdq %%xmm2, %%xmm4 \n\t"
        "movdqa %%xmm1, %%xmm5 \n\t"
        "punpckldq %%xmm3, %%xmm1 \n\t"
        "punpckhdq %%xmm3, %%xmm5 \n\t"
        // Save in transposed order:
        "movups %%xmm0, (%1) \n\t"
        "movups %%xmm4, 16(%1) \n\t"
        "movups %%xmm1, 32(%1) \n\t"
        "movups %%xmm5, 48(%1) \n\t"
        : /* no output operands -- implicitly volatile */
        : "r" (a), "r" (b)
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory" );

    }

    void twofifths_sse2(uint32_t* a, uint64_t* b) {

        asm (
        "movups (%0), %%xmm0 \n\t"
        "movups 16(%0), %%xmm1 \n\t"
        "movdqa %%xmm0, %%xmm2 \n\t"
        "punpcklbw %%xmm1, %%xmm0 \n\t"
        "punpckhbw %%xmm1, %%xmm2 \n\t"
        "movdqa %%xmm0, %%xmm1 \n\t"
        "punpcklbw %%xmm2, %%xmm0 \n\t"
        "punpckhbw %%xmm2, %%xmm1 \n\t"
        "movdqa %%xmm0, %%xmm2 \n\t"
        "punpcklbw %%xmm1, %%xmm0 \n\t"
        "punpckhbw %%xmm1, %%xmm2 \n\t"
        "movups %%xmm0, (%1) \n\t"
        "movups %%xmm2, 16(%1) \n\t"
        : /* no output operands -- implicitly volatile */
        : "r" (a), "r" (b)
        : "xmm0", "xmm1", "xmm2", "memory" );

    }

    void twofifths_avx(uint32_t* a, uint64_t* b) {

        asm (
        "vmovups (%0), %%xmm0 \n\t"
        "vmovups 16(%0), %%xmm1 \n\t"
        "vpunpcklbw %%xmm1, %%xmm0, %%xmm2 \n\t"
        "vpunpckhbw %%xmm1, %%xmm0, %%xmm3 \n\t"
        "vpunpcklbw %%xmm3, %%xmm2, %%xmm0 \n\t"
        "vpunpckhbw %%xmm3, %%xmm2, %%xmm1 \n\t"
        "vpunpcklbw %%xmm1, %%xmm0, %%xmm2 \n\t"
        "vpunpckhbw %%xmm1, %%xmm0, %%xmm3 \n\t"
        "vmovups %%xmm2, (%1) \n\t"
        "vmovups %%xmm3, 16(%1) \n\t"
        : /* no output operands -- implicitly volatile */
        : "r" (a), "r" (b)
        : "xmm0", "xmm1", "xmm2", "xmm3", "memory" );

    }

    void z64_to_r32_sse2(uint64_t* a, uint32_t* b) {
        /*
        * Converts a Z-ordered array of 16 uint64s, each of which encodes
        * an 8-by-8 subsquare of a 32-by-32 square, into an array of 32
        * uint32s, each of which represents a row.
        */

        asm (
        // Load bytes 0 -- 63 into registers:
        "movups (%0), %%xmm0 \n\t"
        "movups 16(%0), %%xmm3 \n\t"
        "movups 32(%0), %%xmm1 \n\t"
        "movups 48(%0), %%xmm4 \n\t"

        // Bit cycle, round I:
        "movdqa %%xmm0, %%xmm2 \n\t"
        "movdqa %%xmm3, %%xmm5 \n\t"
        "punpcklbw %%xmm1, %%xmm0 \n\t"
        "punpcklbw %%xmm4, %%xmm3 \n\t"
        "punpckhbw %%xmm1, %%xmm2 \n\t"
        "punpckhbw %%xmm4, %%xmm5 \n\t"

        // Bit cycle, round II:
        "movdqa %%xmm0, %%xmm1 \n\t"
        "movdqa %%xmm3, %%xmm4 \n\t"
        "punpcklbw %%xmm2, %%xmm0 \n\t"
        "punpcklbw %%xmm5, %%xmm3 \n\t"
        "punpckhbw %%xmm2, %%xmm1 \n\t"
        "punpckhbw %%xmm5, %%xmm4 \n\t"

        // Save bytes 0 -- 63 back into memory:
        "movups %%xmm0, 0(%1) \n\t"
        "movups %%xmm1, 16(%1) \n\t"
        "movups %%xmm3, 32(%1) \n\t"
        "movups %%xmm4, 48(%1) \n\t"

        // Load bytes 64 -- 127 into registers:
        "movups 64(%0), %%xmm0 \n\t"
        "movups 80(%0), %%xmm3 \n\t"
        "movups 96(%0), %%xmm1 \n\t"
        "movups 112(%0), %%xmm4 \n\t"

        // Bit cycle, round I:
        "movdqa %%xmm0, %%xmm2 \n\t"
        "movdqa %%xmm3, %%xmm5 \n\t"
        "punpcklbw %%xmm1, %%xmm0 \n\t"
        "punpcklbw %%xmm4, %%xmm3 \n\t"
        "punpckhbw %%xmm1, %%xmm2 \n\t"
        "punpckhbw %%xmm4, %%xmm5 \n\t"

        // Bit cycle, round II:
        "movdqa %%xmm0, %%xmm1 \n\t"
        "movdqa %%xmm3, %%xmm4 \n\t"
        "punpcklbw %%xmm2, %%xmm0 \n\t"
        "punpcklbw %%xmm5, %%xmm3 \n\t"
        "punpckhbw %%xmm2, %%xmm1 \n\t"
        "punpckhbw %%xmm5, %%xmm4 \n\t"

        // Save bytes 64 -- 127 back into memory:
        "movups %%xmm0, 64(%1) \n\t"
        "movups %%xmm1, 80(%1) \n\t"
        "movups %%xmm3, 96(%1) \n\t"
        "movups %%xmm4, 112(%1) \n\t"

        : /* no output operands -- implicitly volatile */
        : "r" (a), "r" (b)
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory" );

    }

    void z64_to_r32_avx(uint64_t* a, uint32_t* b) {

        asm (
        // Load bytes 0 -- 63 into registers:
        "vmovups (%0), %%xmm0 \n\t"
        "vmovups 16(%0), %%xmm3 \n\t"
        "vmovups 32(%0), %%xmm1 \n\t"
        "vmovups 48(%0), %%xmm4 \n\t"

        // Bit cycle, round I:
        "vpunpckhbw %%xmm1, %%xmm0, %%xmm2 \n\t"
        "vpunpckhbw %%xmm4, %%xmm3, %%xmm5 \n\t"
        "vpunpcklbw %%xmm1, %%xmm0, %%xmm0 \n\t"
        "vpunpcklbw %%xmm4, %%xmm3, %%xmm3 \n\t"

        // Bit cycle, round II:
        "vpunpckhbw %%xmm2, %%xmm0, %%xmm1 \n\t"
        "vpunpckhbw %%xmm5, %%xmm3, %%xmm4 \n\t"
        "vpunpcklbw %%xmm2, %%xmm0, %%xmm0 \n\t"
        "vpunpcklbw %%xmm5, %%xmm3, %%xmm3 \n\t"

        // Save bytes 0 -- 63 back into memory:
        "vmovups %%xmm0, 0(%1) \n\t"
        "vmovups %%xmm1, 16(%1) \n\t"
        "vmovups %%xmm3, 32(%1) \n\t"
        "vmovups %%xmm4, 48(%1) \n\t"

        // Load bytes 64 -- 127 into registers:
        "vmovups 64(%0), %%xmm0 \n\t"
        "vmovups 80(%0), %%xmm3 \n\t"
        "vmovups 96(%0), %%xmm1 \n\t"
        "vmovups 112(%0), %%xmm4 \n\t"

        // Bit cycle, round I:
        "vpunpckhbw %%xmm1, %%xmm0, %%xmm2 \n\t"
        "vpunpckhbw %%xmm4, %%xmm3, %%xmm5 \n\t"
        "vpunpcklbw %%xmm1, %%xmm0, %%xmm0 \n\t"
        "vpunpcklbw %%xmm4, %%xmm3, %%xmm3 \n\t"

        // Bit cycle, round II:
        "vpunpckhbw %%xmm2, %%xmm0, %%xmm1 \n\t"
        "vpunpckhbw %%xmm5, %%xmm3, %%xmm4 \n\t"
        "vpunpcklbw %%xmm2, %%xmm0, %%xmm0 \n\t"
        "vpunpcklbw %%xmm5, %%xmm3, %%xmm3 \n\t"

        // Save bytes 64 -- 127 back into memory:
        "vmovups %%xmm0, 64(%1) \n\t"
        "vmovups %%xmm1, 80(%1) \n\t"
        "vmovups %%xmm3, 96(%1) \n\t"
        "vmovups %%xmm4, 112(%1) \n\t"

        : /* no output operands -- implicitly volatile */
        : "r" (a), "r" (b)
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory" );

    }

    void z64_to_r32_avx2(uint64_t* a, uint32_t* b) {

        asm (
        // Load memory into registers:
        "vmovups (%0), %%ymm0 \n\t"
        "vmovups 32(%0), %%ymm1 \n\t"
        "vmovups 64(%0), %%ymm2 \n\t"
        "vmovups 96(%0), %%ymm3 \n\t"

        // Bit cycle, round I:
        "vpunpcklbw %%ymm1, %%ymm0, %%ymm4 \n\t"
        "vpunpckhbw %%ymm1, %%ymm0, %%ymm5 \n\t"
        "vpunpcklbw %%ymm3, %%ymm2, %%ymm6 \n\t"
        "vpunpckhbw %%ymm3, %%ymm2, %%ymm7 \n\t"

        // Exchange between low and high halves of ymm registers:
        "vpermq $216, %%ymm4, %%ymm0 \n\t"
        "vpermq $216, %%ymm5, %%ymm1 \n\t"
        "vpermq $216, %%ymm6, %%ymm2 \n\t"
        "vpermq $216, %%ymm7, %%ymm3 \n\t"

        // Bit cycle, round II:
        "vpunpcklbw %%ymm1, %%ymm0, %%ymm4 \n\t"
        "vpunpckhbw %%ymm1, %%ymm0, %%ymm5 \n\t"
        "vpunpcklbw %%ymm3, %%ymm2, %%ymm6 \n\t"
        "vpunpckhbw %%ymm3, %%ymm2, %%ymm7 \n\t"

        // Save registers back into memory:
        "vmovups %%ymm4, 0(%1) \n\t"
        "vmovups %%ymm5, 32(%1) \n\t"
        "vmovups %%ymm6, 64(%1) \n\t"
        "vmovups %%ymm7, 96(%1) \n\t"

        : /* no output operands -- implicitly volatile */
        : "r" (a), "r" (b)
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "memory" );

    }

    void z64_to_r32_centre_ssse3(uint64_t* c, uint32_t* b) {
        /*
        *    #ab#
        *    #cd# <--- [a, b, c, d]
        */

        asm (

        // Load from memory:
        "movups (%0), %%xmm0 \n\t"
        "movups 16(%0), %%xmm2 \n\t"

        // Permute bytes:
        "pshufb (%2), %%xmm0 \n\t"
        "pshufb (%2), %%xmm2 \n\t"

        // Dirty hack to perform << 8 and >> 8 during movups:
        "movups %%xmm0, 1(%1) \n\t"
        "movups %%xmm0, 15(%1) \n\t"
        "movups %%xmm2, 33(%1) \n\t"
        "movups %%xmm2, 47(%1) \n\t"

        : /* no output operands -- implicitly volatile */
        : "r" (c), "r" (b), "r" (__linvperm)
        : "xmm0", "xmm1", "xmm2", "xmm3", "memory" );
    }

    void z64_to_r32_centre_avx(uint64_t* c, uint32_t* b) {
        /*
        *    #ab#
        *    #cd# <--- [a, b, c, d]
        */

        asm (

        // Load from memory:
        "vmovups (%0), %%xmm0 \n\t"
        "vmovups 16(%0), %%xmm2 \n\t"

        // Permute bytes:
        "vpshufb (%2), %%xmm0, %%xmm0 \n\t"
        "vpshufb (%2), %%xmm2, %%xmm2 \n\t"

        // Dirty hack to perform << 8 and >> 8 during movups:
        "vmovups %%xmm0, 1(%1) \n\t"
        "vmovups %%xmm0, 15(%1) \n\t"
        "vmovups %%xmm2, 33(%1) \n\t"
        "vmovups %%xmm2, 47(%1) \n\t"

        : /* no output operands -- implicitly volatile */
        : "r" (c), "r" (b), "r" (__linvperm)
        : "xmm0", "xmm1", "xmm2", "xmm3", "memory" );
    }

    void r32_centre_to_z64_ssse3(uint32_t* b, uint64_t* c) {
        /*
        * Selects the 16-by-16 square in the centre of a 32-by-32
        * square encoded as an array of rows, and converts it to a
        * Z-ordered array of 4 uint64s, each representing a 8-by-8
        * subsquare:
        *
        *    ####
        *    #ab#
        *    #cd# ---> [a, b, c, d]
        *    ####
        */

        asm (
        // Dirty hack to perform << 8 and >> 8 during movups:
        "movups 31(%1), %%xmm0 \n\t"
        "movups 49(%1), %%xmm1 \n\t"
        "movups 63(%1), %%xmm2 \n\t"
        "movups 81(%1), %%xmm3 \n\t"
        "psrld $16, %%xmm0 \n\t"
        "pslld $16, %%xmm1 \n\t"
        "psrld $16, %%xmm2 \n\t"
        "pslld $16, %%xmm3 \n\t"

        // Alternately select words from two registers:
        "por %%xmm1, %%xmm0 \n\t"
        "por %%xmm3, %%xmm2 \n\t"

        // Permute bytes:
        "pshufb (%2), %%xmm0 \n\t"
        "pshufb (%2), %%xmm2 \n\t"

        // Save back into memory:
        "movups %%xmm0, (%0) \n\t"
        "movups %%xmm2, 16(%0) \n\t"

        : /* no output operands -- implicitly volatile */
        : "r" (c), "r" (b), "r" (__lifeperm)
        : "xmm0", "xmm1", "xmm2", "xmm3", "memory" );

    }

    void r32_centre_to_z64_sse4(uint32_t* b, uint64_t* c) {

        asm (
        // Dirty hack to perform << 8 and >> 8 during movups:
        "movups 33(%1), %%xmm0 \n\t"
        "movups 47(%1), %%xmm1 \n\t"
        "movups 65(%1), %%xmm2 \n\t"
        "movups 79(%1), %%xmm3 \n\t"

        // Alternately select words from two registers:
        "pblendw $170, %%xmm1, %%xmm0 \n\t"
        "pblendw $170, %%xmm3, %%xmm2 \n\t"

        // Permute bytes:
        "pshufb (%2), %%xmm0 \n\t"
        "pshufb (%2), %%xmm2 \n\t"

        // Save back into memory:
        "movups %%xmm0, (%0) \n\t"
        "movups %%xmm2, 16(%0) \n\t"

        : /* no output operands -- implicitly volatile */
        : "r" (c), "r" (b), "r" (__lifeperm)
        : "xmm0", "xmm1", "xmm2", "xmm3", "memory" );

    }

    void r32_centre_to_z64_avx(uint32_t* b, uint64_t* c) {

        asm (
        // Dirty hack to perform << 8 and >> 8 during movups:
        "vmovups 33(%1), %%xmm0 \n\t"
        "vmovups 47(%1), %%xmm1 \n\t"
        "vmovups 65(%1), %%xmm2 \n\t"
        "vmovups 79(%1), %%xmm3 \n\t"

        // Alternately select words from two registers:
        "vpblendw $170, %%xmm1, %%xmm0, %%xmm0 \n\t"
        "vpblendw $170, %%xmm3, %%xmm2, %%xmm2 \n\t"

        // Permute bytes:
        "vpshufb (%2), %%xmm0, %%xmm0 \n\t"
        "vpshufb (%2), %%xmm2, %%xmm2 \n\t"

        // Save back into memory:
        "vmovups %%xmm0, (%0) \n\t"
        "vmovups %%xmm2, 16(%0) \n\t"

        : /* no output operands -- implicitly volatile */
        : "r" (c), "r" (b), "r" (__lifeperm)
        : "xmm0", "xmm1", "xmm2", "xmm3", "memory" );

    }

    void r32_centre_to_z64_avx2(uint32_t* b, uint64_t* c) {

        asm (
        // Only 7 instructions -- and 4 are memory operations:
        "vmovups 33(%1), %%ymm0 \n\t"
        "vmovups 63(%1), %%ymm1 \n\t"
        "vmovups 32(%2), %%ymm2 \n\t"
        "vpblendw $170, %%ymm1, %%ymm0, %%ymm0 \n\t"
        "vpshufb (%2), %%ymm0, %%ymm0 \n\t"
        "vpermd %%ymm0, %%ymm2, %%ymm0 \n\t"
        "vmovups %%ymm0, (%0) \n\t"

        : /* no output operands -- implicitly volatile */
        : "r" (c), "r" (b), "r" (__lifeperm)
        : "xmm0", "xmm1", "xmm2", "memory" );

    }

}


