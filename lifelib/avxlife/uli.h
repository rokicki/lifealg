#pragma once
#include <stdint.h>
#include <cstring>
#include <iostream>
#include "lifeperm.h"
#include "lifelogic/iterators_all.h"

namespace apg {


    template<int N>
    int universal_leaf_iterator(int rule, int history, uint64_t mantissa, uint64_t **inleafxs, uint64_t *outleafx) {

                int nochange = -1;

                uint64_t inleaves[16];
                for (int i = 0; i < 4; i++) {
                    std::memcpy(inleaves+(4*i), inleafxs[i], 32);
                }

                // Apply our vectorised bitwise life algorithm:
                if (history == 0) {
                    // RULE FAMILY: Life
                    nochange = iterate_var_leaf(rule, mantissa, inleaves, outleafx);
                } else if (history == 1) {
                    // RULE FAMILY: LifeHistory
                    uint64_t hleaves[16];
                    for (int i = 0; i < 4; i++) {
                        std::memcpy(hleaves+(4*i), inleafxs[i] + 4, 32);
                    }
                    if (N >= 3) {
                        uint64_t jleaves[16];
                        for (int i = 0; i < 4; i++) {
                            std::memcpy(jleaves+(4*i), inleafxs[i] + 8, 32);
                        }
                        nochange = iterate_var_leaf(rule, mantissa, inleaves, hleaves, jleaves, outleafx);
                        // Copy annotation layers:
                        for (int j = 3; j < N; j++) {
                            for (int i = 0; i < 4; i++) {
                                outleafx[4*j+i] = inleafxs[i][4*j+3-i];
                            }
                        }
                    } else {
                        nochange = iterate_var_leaf(rule, mantissa, inleaves, hleaves, outleafx);
                    }
                } else if ((history == 2) || (history == 3)) {
                    // RULE FAMILY: Generations:
                    uint64_t hleaves[16];
                    for (int i = 0; i < 4; i++) {
                        std::memcpy(hleaves+(4*i), inleafxs[i] + 4, 32);
                    }
                    // Copy annotation layers:
                    for (int j = 0; j < N; j++) {
                        for (int i = 0; i < 4; i++) {
                            outleafx[4*j+i] = inleafxs[i][4*j+3-i];
                        }
                    }
                    if (history == 3) {
                        for (int i = 0; i < 4; i++) {
                            outleafx[4*(N-1)+i] |= outleafx[i];
                        }
                    }
                    if (mantissa) {
                        nochange = iterate_var_leaf(rule, mantissa, inleaves, hleaves, outleafx);
                    }
                    if (history == 3) {
                        for (int j = 0; j < N-1; j++) {
                            for (int i = 0; i < 4; i++) {
                                outleafx[4*(N-1)+i] |= outleafx[4*j+i];
                            }
                        }
                    }
                } else if ((history == 4) || (history == 5)) {
                    // Copy annotation layers:
                    for (int j = 0; j < N; j++) {
                        for (int i = 0; i < 4; i++) {
                            outleafx[4*j+i] = inleafxs[i][4*j+3-i];
                        }
                    }
                    if (mantissa) {
                        nochange = iterate_var_leaf(rule, mantissa, inleaves, outleafx);
                    }
                    if (history == 5) {
                        for (int j = 0; j < N-1; j++) {
                            for (int i = 0; i < 4; i++) {
                                outleafx[4*(N-1)+i] |= outleafx[4*j+i];
                            }
                        }
                    }
                }
                if (mantissa) {
                    return nochange;
                } else {
                    return 0;
                }
    }
}
