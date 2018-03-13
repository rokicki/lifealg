#include "iterators_b3s23.h"
#include "iterators_b36s23.h"
#include <iostream>

int main() {

    uint32_t d[32];
    uint32_t h[32];
    for (int i = 0; i < 32; i++) {
        d[i] = 0; h[i] = 0;
    }
    d[15] = 31 << 14;
    d[16] = 17 << 14;
    std::cout << "--------" << std::endl;
    // clock_t start_time = clock();
    for (int k = 0; k < 10; k++) {
        for (int i = 8; i < 24; i++) {
            uint32_t x = d[i];
            int j = 32;
            while (j --> 0) {
                if (1 & (x >> j)) {
                    std::cout << "*";
                } else {
                    std::cout << ".";
                }
            }
            std::cout << std::endl;
        }
        b3s23::iterate_var_sse2(8, d);
        // std::cout << "Value: " << apg::iter8_sse2(d, h) << std::endl;
    }
    for (int i = 0; i < 32; i++) {
        d[i] = 0; h[i] = 0;
    }
    d[14] = 14 << 14;
    d[15] = 1 << 14;
    d[16] = 1 << 14;
    d[17] = 1 << 14;
    std::cout << "--------" << std::endl;
    // clock_t start_time = clock();
    for (int k = 0; k < 10; k++) {
        for (int i = 8; i < 24; i++) {
            uint32_t x = d[i];
            int j = 32;
            while (j --> 0) {
                if (1 & (x >> j)) {
                    std::cout << "*";
                } else {
                    std::cout << ".";
                }
            }
            std::cout << std::endl;
        }
        b36s23::iterate_var_sse2(8, d);
        // std::cout << "Value: " << apg::iter8_sse2(d, h) << std::endl;
    }
    // clock_t end_time = clock();
    // std::cout << "iter8 in " << ((double) (end_time - start_time) / CLOCKS_PER_SEC) << " us." << std::endl;

    return 0;

}
