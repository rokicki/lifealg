        "movdqa %%xmm11, %%xmm1 \n\t"
        "pand %%xmm8, %%xmm1 \n\t"
        "pxor %%xmm11, %%xmm8 \n\t"
        "pxor %%xmm1, %%xmm9 \n\t"
        "pxor %%xmm9, %%xmm8 \n\t"
        "por %%xmm10, %%xmm12 \n\t"
        "pxor %%xmm9, %%xmm10 \n\t"
        "pand %%xmm12, %%xmm8 \n\t"
        "pand %%xmm8, %%xmm10 \n\t"
