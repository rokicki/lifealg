#include <iostream>
#include "hypertree.h"

int main() {

    typedef apg::nicearray<uint64_t, 4> uint64x4;
    typedef apg::nicearray<uint32_t, 4> uint32x4;
    apg::hypertree<uint32_t, 4, double, uint64x4, double> htree;

    uint32_t k = htree.make_leaf(uint64x4(5, 6, 7, 8));
    std::cout << k << std::endl;

    std::cout << htree.make_nonleaf(uint32x4(0, 0, 0, 0)) << std::endl;
    std::cout << htree.make_nonleaf(uint32x4(0, 0, k, 0)) << std::endl;
    std::cout << htree.make_nonleaf(uint32x4(k, k, k, 0)) << std::endl;
    std::cout << htree.make_nonleaf(uint32x4(0, 0, k, 0)) << std::endl;

    htree.gc_mark(apg::hypernode<uint32_t>(2, 1));

}
