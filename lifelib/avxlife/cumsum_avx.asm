"vmovups (%0), %%xmm0 \n\t"
"vmovups 16(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 16(%0) \n\t"
"vmovups 32(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 32(%0) \n\t"
"vmovups 48(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 48(%0) \n\t"
"vmovups 64(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 64(%0) \n\t"
"vmovups 80(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 80(%0) \n\t"
"vmovups 96(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 96(%0) \n\t"
"vmovups 112(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 112(%0) \n\t"
"vmovups 128(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 128(%0) \n\t"
"vmovups 144(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 144(%0) \n\t"
"vmovups 160(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 160(%0) \n\t"
"vmovups 176(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 176(%0) \n\t"
"vmovups 192(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 192(%0) \n\t"
"vmovups 208(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 208(%0) \n\t"
"vmovups 224(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 224(%0) \n\t"
"vmovups 240(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 240(%0) \n\t"
"vmovups 256(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 256(%0) \n\t"
"vmovups 272(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 272(%0) \n\t"
"vmovups 288(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 288(%0) \n\t"
"vmovups 304(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 304(%0) \n\t"
"vmovups 320(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 320(%0) \n\t"
"vmovups 336(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 336(%0) \n\t"
"vmovups 352(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 352(%0) \n\t"
"vmovups 368(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 368(%0) \n\t"
"vmovups 384(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 384(%0) \n\t"
"vmovups 400(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 400(%0) \n\t"
"vmovups 416(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 416(%0) \n\t"
"vmovups 432(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 432(%0) \n\t"
"vmovups 448(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 448(%0) \n\t"
"vmovups 464(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 464(%0) \n\t"
"vmovups 480(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 480(%0) \n\t"
"vmovups 496(%0), %%xmm1 \n\t"
"vpaddb %%xmm1, %%xmm0, %%xmm0\n\t"
"vmovups %%xmm0, 496(%0) \n\t"
