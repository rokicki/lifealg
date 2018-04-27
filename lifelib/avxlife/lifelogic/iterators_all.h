#pragma once
#include <string>
#include "iterators_b3s23.h"
namespace apg {

    int rule2int(std::string rule) {
        if (rule == "b3s23") { return 0; }
        return -1;
    }

    int iterate_var_leaf(int rule, int n, uint64_t * inleaves, uint64_t * outleaf) {
        switch(rule) {
            case 0 :
                return b3s23::iterate_var_leaf(n, inleaves, outleaf);
        }
        return -1;
    }

    int iterate_var_32_28(int rule, uint32_t* d, uint32_t * diffs) {
        uint32_t e[32];
        int bis = apg::best_instruction_set();
        switch(rule) {
            case 0 :
                if (bis >= 10) {
                    return b3s23::iterate_avx2_32_28(d, e, 0, 0, diffs, false);
                } else if (bis >= 9) {
                    return b3s23::iterate_avx_32_28(d, e, 0, 0, diffs, false);
                } else {
                    return b3s23::iterate_sse2_32_28(d, e, 0, 0, diffs, false);
                }
        }
        return -1;
    }

    int iterate_var_leaf(int rule, int n, uint64_t * inleaves, uint64_t * hleaves, uint64_t * outleaf) {
        switch(rule) {
            case 0 :
                return b3s23::iterate_var_leaf(n, inleaves, hleaves, outleaf);
        }
        return -1;
    }

    int iterate_var_32_28(int rule, uint32_t* d, uint32_t* h, uint32_t * diffs) {
        uint32_t e[32];
        int bis = apg::best_instruction_set();
        switch(rule) {
            case 0 :
                if (bis >= 10) {
                    return b3s23::iterate_avx2_32_28(d, e, h, 0, diffs, false);
                } else if (bis >= 9) {
                    return b3s23::iterate_avx_32_28(d, e, h, 0, diffs, false);
                } else {
                    return b3s23::iterate_sse2_32_28(d, e, h, 0, diffs, false);
                }
        }
        return -1;
    }

    int iterate_var_leaf(int rule, int n, uint64_t * inleaves, uint64_t * hleaves, uint64_t * jleaves, uint64_t * outleaf) {
        switch(rule) {
            case 0 :
                return b3s23::iterate_var_leaf(n, inleaves, hleaves, jleaves, outleaf);
        }
        return -1;
    }

    int iterate_var_32_28(int rule, uint32_t* d, uint32_t* h, uint32_t* j, uint32_t * diffs) {
        uint32_t e[32];
        int bis = apg::best_instruction_set();
        switch(rule) {
            case 0 :
                if (bis >= 10) {
                    return b3s23::iterate_avx2_32_28(d, e, h, j, diffs, false);
                } else if (bis >= 9) {
                    return b3s23::iterate_avx_32_28(d, e, h, j, diffs, false);
                } else {
                    return b3s23::iterate_sse2_32_28(d, e, h, j, diffs, false);
                }
        }
        return -1;
    }

    int uli_get_family(int rule) {
        switch (rule) {
            case 0 :
                return 0;
        }
        return 0;
    }

    uint64_t uli_valid_mantissa(int rule) {
        switch (rule) {
            case 0 :
                return 511;
        }
        return 3;
    }

}
