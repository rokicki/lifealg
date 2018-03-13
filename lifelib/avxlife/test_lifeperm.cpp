#include <iostream>
#include <stdint.h>
#include "lifeperm.h"
#include "eors.h"

int main() {

    // std::cout << "Best instruction set: " << apg::best_instruction_set() << std::endl;
    std::cout << "CPU name: " << apg::cpu_name() << std::endl;

    uint64_t a[16];
    for (int i = 0; i < 16; i++) {
        a[i] = 0x0807060504030201ull + (0x0808080808080808ull * i);
    }
    uint32_t b1[32];
    uint32_t b2[32];
    uint32_t b3[32];
    uint64_t c1[4];
    uint64_t c2[4];
    uint64_t c3[4];

    apg::z64_to_r32_sse2(a, b1);
    apg::z64_to_r32_sse2(a, b2);
    // apg::z64_to_r32_avx2(a, b3);

    for (int i = 0; i < 32; i++) {
        uint32_t x = b1[i];
        uint32_t y = b2[i];
        // uint32_t z = b3[i];
        for (int j = 0; j < 4; j++) {
            std::cout << (x & 0xff) << ' '; x = x >> 8;
            std::cout << (y & 0xff) << ' '; y = y >> 8;
            // std::cout << (z & 0xff) << ' '; z = z >> 8;
        }
        std::cout << std::endl;
    }

    apg::r32_centre_to_z64_sse4(b1, c1);
    apg::r32_centre_to_z64_ssse3(b2, c2);
    // apg::r32_centre_to_z64_avx2(b3, c3);

    for (int i = 0; i < 4; i++) {
        uint64_t x = c1[i];
        uint64_t y = c2[i];
        // uint64_t z = c3[i];
        for (int j = 0; j < 8; j++) {
            std::cout << (x & 0xff) << ' '; x = x >> 8;
            std::cout << (y & 0xff) << ' '; y = y >> 8;
            // std::cout << (z & 0xff) << ' '; z = z >> 8;
        }
        std::cout << std::endl;
    }

    uint8_t d[64];
    for (int i = 0; i < 64; i++) { d[i] = i + 1; }
    uint8_t e[64];
    apg::transpose_bytes_sse2((uint64_t*) d, (uint64_t*) e);
    for (int i = 0; i < 64; i++) {
        std::cout << ((int) e[i]) << " ";
        if (i % 8 == 7) { std::cout << std::endl; }
    }

    return 0;

}
