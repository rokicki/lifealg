#include "ltl.h"
#include <iostream>

int main() {

    uint8_t a[1024];
    uint8_t b[256];
    uint8_t c[256];

    for (uint64_t i = 0; i < 1024; i++) { a[i] = ((i == 300) || (i == 303) || (i == 200)) ? 1 : 0; }
    /*
    apg::transpose_16x16_sse2(a, b, true);
    for (uint64_t i = 0; i < 512; i++) { std::cout << ((int) a[i]) << " "; if (i % 16 == 15) { std::cout << std::endl; } }
    std::cout << std::endl;
    apg::inplace_cumsum_sse2(a);
    for (uint64_t i = 0; i < 512; i++) { std::cout << ((int) a[i]) << " "; if (i % 16 == 15) { std::cout << std::endl; } }
    std::cout << std::endl;
    */
    // apg::convolve2d((uint64_t*) a, (uint64_t*) b, 3);
    apg::ntsum2d((uint64_t*) a, (uint64_t*) b);
    for (uint64_t i = 0; i < 256; i++) { std::cout << ((int) b[i]) << " "; if (i % 16 == 15) { std::cout << std::endl; } }

    uint64_t x[4] = {0x0123456789abcdefull, 0, 0x0011223344556677ull, 0x8899aabbccddeeffull};
    apg::bits2bytes(x, (uint64_t*) b);
    apg::bits2bytes_sse2(x, (uint64_t*) c);
    std::cout << std::endl;
    for (uint64_t i = 0; i < 256; i++) { std::cout << ((int) b[i]) << " "; if (i % 16 == 15) { std::cout << std::endl; } }
    std::cout << std::endl;
    for (uint64_t i = 0; i < 256; i++) { std::cout << ((int) c[i]) << " "; if (i % 16 == 15) { std::cout << std::endl; } }

    return 0;

}
