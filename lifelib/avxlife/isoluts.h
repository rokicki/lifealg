
    uint8_t* lut18 = 0;
    uint8_t* lut24 = 0;
    uint32_t* mix18 = 0;

    uint8_t xfl9(uint64_t i) {
        return ((lut9[((i >> 6) & 7) | ((i >> 9) & 56)] >> (i & 7)) & 1);
    }

    void assemble_lut18() {
        std::cerr << "Computing 2^18-entry lookup table...";
        lut18 = (uint8_t*) std::malloc(0x40000); // don't bother freeing; the OS will do that.
        for (uint64_t i = 0; i < 0x40000; i++) {
            lut18[i] = xfl9(i) | (xfl9(i >> 1) << 1) | (xfl9(i >> 2) << 2) | (xfl9(i >> 3) << 3);
        }
        std::cerr << "done!" << std::endl;
    }

    void assemble_lut24() {
        if (lut18 == 0) { assemble_lut18(); }
        std::cerr << "Computing 2^24-entry lookup table...";
        lut24 = (uint8_t*) std::malloc(0x1000000); // don't bother freeing; the OS will do that.
        for (uint64_t i = 0; i < 0x1000000; i++) {
            lut24[i] = lut18[i & 0x3ffff] | (lut18[i >> 6] << 4);
        }
        std::cerr << "done!" << std::endl;
    }

    void assemble_mix18() {
        std::cerr << "Computing 2^18-entry mixing table...";
        mix18 = (uint32_t*) std::malloc(0x100000); // see above.
        for (uint64_t i = 0; i < 0x3ffff; i++) {
            mix18[i] = (i & 0x3f) | (((i >> 4) & 0x3f) << 8) | (((i >> 8) & 0x3f) << 16) | (((i >> 12) & 0x3f) << 24);
        }
        std::cerr << "done!" << std::endl;
    }

    bool iterate_var_leaf(uint64_t *inleaves, uint64_t *outleafx) {

        if (lut24 == 0) { assemble_lut24(); }
        if (mix18 == 0) { assemble_mix18(); }

        uint32_t d[32];
        uint32_t e[32];
        uint32_t f[32];
        uint8_t  g[32];
        apg::z64_to_r32_sse2(inleaves, d);

        for (uint64_t i = 7; i < 25; i++) { f[i] = mix18[(d[i] >> 7) & 0x3ffff]; }

        asm (
        // Load data from memory into registers:
        "movups (%1), %%xmm1 \n\t"
        "movups 16(%1), %%xmm2 \n\t"
        "movups 32(%1), %%xmm3 \n\t"

        "movups 28(%0), %%xmm0 \n\t"
        "pshufb %%xmm1, %%xmm0 \n\t"
        "movdqa %%xmm2, %%xmm4 \n\t"
        "pandn %%xmm0, %%xmm4 \n\t"
        "psrld $2, %%xmm4 \n\t"
        "pand %%xmm2, %%xmm0 \n\t"
        "por %%xmm4, %%xmm0 \n\t"
        "movdqa %%xmm3, %%xmm4 \n\t"
        "pandn %%xmm0, %%xmm4 \n\t"
        "psrld $4, %%xmm4 \n\t"
        "pand %%xmm3, %%xmm0 \n\t"
        "por %%xmm4, %%xmm0 \n\t"
        "movups %%xmm0, (%2) \n\t"

        "movups 36(%0), %%xmm0 \n\t"
        "pshufb %%xmm1, %%xmm0 \n\t"
        "movdqa %%xmm2, %%xmm4 \n\t"
        "pandn %%xmm0, %%xmm4 \n\t"
        "psrld $2, %%xmm4 \n\t"
        "pand %%xmm2, %%xmm0 \n\t"
        "por %%xmm4, %%xmm0 \n\t"
        "movdqa %%xmm3, %%xmm4 \n\t"
        "pandn %%xmm0, %%xmm4 \n\t"
        "psrld $4, %%xmm4 \n\t"
        "pand %%xmm3, %%xmm0 \n\t"
        "por %%xmm4, %%xmm0 \n\t"
        "movups %%xmm0, 16(%2) \n\t"

        "movups 44(%0), %%xmm0 \n\t"
        "pshufb %%xmm1, %%xmm0 \n\t"
        "movdqa %%xmm2, %%xmm4 \n\t"
        "pandn %%xmm0, %%xmm4 \n\t"
        "psrld $2, %%xmm4 \n\t"
        "pand %%xmm2, %%xmm0 \n\t"
        "por %%xmm4, %%xmm0 \n\t"
        "movdqa %%xmm3, %%xmm4 \n\t"
        "pandn %%xmm0, %%xmm4 \n\t"
        "psrld $4, %%xmm4 \n\t"
        "pand %%xmm3, %%xmm0 \n\t"
        "por %%xmm4, %%xmm0 \n\t"
        "movups %%xmm0, 32(%2) \n\t"

        "movups 52(%0), %%xmm0 \n\t"
        "pshufb %%xmm1, %%xmm0 \n\t"
        "movdqa %%xmm2, %%xmm4 \n\t"
        "pandn %%xmm0, %%xmm4 \n\t"
        "psrld $2, %%xmm4 \n\t"
        "pand %%xmm2, %%xmm0 \n\t"
        "por %%xmm4, %%xmm0 \n\t"
        "movdqa %%xmm3, %%xmm4 \n\t"
        "pandn %%xmm0, %%xmm4 \n\t"
        "psrld $4, %%xmm4 \n\t"
        "pand %%xmm3, %%xmm0 \n\t"
        "por %%xmm4, %%xmm0 \n\t"
        "movups %%xmm0, 48(%2) \n\t"

        "movups 60(%0), %%xmm0 \n\t"
        "pshufb %%xmm1, %%xmm0 \n\t"
        "movdqa %%xmm2, %%xmm4 \n\t"
        "pandn %%xmm0, %%xmm4 \n\t"
        "psrld $2, %%xmm4 \n\t"
        "pand %%xmm2, %%xmm0 \n\t"
        "por %%xmm4, %%xmm0 \n\t"
        "movdqa %%xmm3, %%xmm4 \n\t"
        "pandn %%xmm0, %%xmm4 \n\t"
        "psrld $4, %%xmm4 \n\t"
        "pand %%xmm3, %%xmm0 \n\t"
        "por %%xmm4, %%xmm0 \n\t"
        "movups %%xmm0, 64(%2) \n\t"

        "movups 68(%0), %%xmm0 \n\t"
        "pshufb %%xmm1, %%xmm0 \n\t"
        "movdqa %%xmm2, %%xmm4 \n\t"
        "pandn %%xmm0, %%xmm4 \n\t"
        "psrld $2, %%xmm4 \n\t"
        "pand %%xmm2, %%xmm0 \n\t"
        "por %%xmm4, %%xmm0 \n\t"
        "movdqa %%xmm3, %%xmm4 \n\t"
        "pandn %%xmm0, %%xmm4 \n\t"
        "psrld $4, %%xmm4 \n\t"
        "pand %%xmm3, %%xmm0 \n\t"
        "por %%xmm4, %%xmm0 \n\t"
        "movups %%xmm0, 80(%2) \n\t"

        "movups 76(%0), %%xmm0 \n\t"
        "pshufb %%xmm1, %%xmm0 \n\t"
        "movdqa %%xmm2, %%xmm4 \n\t"
        "pandn %%xmm0, %%xmm4 \n\t"
        "psrld $2, %%xmm4 \n\t"
        "pand %%xmm2, %%xmm0 \n\t"
        "por %%xmm4, %%xmm0 \n\t"
        "movdqa %%xmm3, %%xmm4 \n\t"
        "pandn %%xmm0, %%xmm4 \n\t"
        "psrld $4, %%xmm4 \n\t"
        "pand %%xmm3, %%xmm0 \n\t"
        "por %%xmm4, %%xmm0 \n\t"
        "movups %%xmm0, 96(%2) \n\t"

        "movups 84(%0), %%xmm0 \n\t"
        "pshufb %%xmm1, %%xmm0 \n\t"
        "movdqa %%xmm2, %%xmm4 \n\t"
        "pandn %%xmm0, %%xmm4 \n\t"
        "psrld $2, %%xmm4 \n\t"
        "pand %%xmm2, %%xmm0 \n\t"
        "por %%xmm4, %%xmm0 \n\t"
        "movdqa %%xmm3, %%xmm4 \n\t"
        "pandn %%xmm0, %%xmm4 \n\t"
        "psrld $4, %%xmm4 \n\t"
        "pand %%xmm3, %%xmm0 \n\t"
        "por %%xmm4, %%xmm0 \n\t"
        "movups %%xmm0, 112(%2) \n\t"

        : /* no output operands -- implicitly volatile */
        : "r" (f), "r" (lut9 + 64), "r" (e)
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "memory" );

        for (uint64_t i = 0; i < 32; i++) { g[i] = lut24[e[i]]; }
        std::memcpy(outleafx, g, 32);

        for (uint64_t i = 0; i < 4; i++) {
            outleafx[i] = (outleafx[i] & 0xf00ff00ff00ff00full) |
                         ((outleafx[i] & 0x00f000f000f000f0ull) << 4) |
                         ((outleafx[i] & 0x0f000f000f000f00ull) >> 4);
            outleafx[i] = (outleafx[i] & 0xffff00000000ffffull) |
                         ((outleafx[i] & 0x00000000ffff0000ull) << 16) |
                         ((outleafx[i] & 0x0000ffff00000000ull) >> 16);
        }

        asm (
        "movups (%0), %%xmm0 \n\t"
        "shufps $216, %%xmm0, %%xmm0 \n\t"
        "movups %%xmm0, (%0) \n\t"
        "movups 16(%0), %%xmm0 \n\t"
        "shufps $216, %%xmm0, %%xmm0 \n\t"
        "movups %%xmm0, 16(%0) \n\t"
        : /* no output operands -- implicitly volatile */
        : "r" (outleafx)
        : "xmm0", "memory" );

        return false;
    }
