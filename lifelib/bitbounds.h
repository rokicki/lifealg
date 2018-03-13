#pragma once
#include <stdint.h>

namespace apg {

    uint64_t upset(uint64_t x) {
        uint64_t y = x;
        y |= ((y & 0x5555555555555555) << 1);
        y |= ((y & 0x3333333333333333) << 2);
        y |= ((y & 0x0f0f0f0f0f0f0f0f) << 4);
        y |= ((y & 0x00ff00ff00ff00ff) << 8);
        y |= ((y & 0x0000ffff0000ffff) << 16);
        y |= ((y & 0x00000000ffffffff) << 32);
        return y;
    }

    void uint64_convolve2(uint64_t a, uint64_t b, uint64_t *out, bool exclusive) {
        // Convolution (works best when b is sparse):
        uint64_t brem = b;
        while (brem) {
            uint64_t c = (brem & (-brem)); // extract a single bit from brem
            brem ^= c; // remove bit
            uint64_t tzc = __builtin_ctzll(c); // determine shifts
            uint64_t xs = tzc & 7;
            uint64_t ys = tzc & 56;
            uint64_t bitmask = (0x0101010101010101ull << xs) - 0x0101010101010101ull;
            uint64_t right = (a >> (8 - xs)) & bitmask;
            uint64_t left = (a << xs) & (~bitmask);
            if (exclusive) {
                out[0] ^= (left << ys);
                out[1] ^= (right << ys);
                if (ys) {
                    out[2] ^= (left >> (64 - ys));
                    out[3] ^= (right >> (64 - ys));
                }
            } else {
                out[0] |= (left << ys);
                out[1] |= (right << ys);
                if (ys) {
                    out[2] |= (left >> (64 - ys));
                    out[3] |= (right >> (64 - ys));
                }
            }
        }
    }

    void uint64_convolve(uint64_t a, uint64_t b, uint64_t *out, bool exclusive) {
        // Convolve two 8-by-8 squares to produce a 16-by-16 result:
        if (__builtin_popcountll(a) > __builtin_popcountll(b)) {
            uint64_convolve2(a, b, out, exclusive);
        } else {
            uint64_convolve2(b, a, out, exclusive);
        }
    }

    uint64_t uint64_bottom(uint64_t tile) {
        uint64_t dy = 0;
        if (tile & 0xff00000000000000ull) {
            dy = 7;
        } else if (tile & 0x00ff000000000000ull) {
            dy = 6;
        } else if (tile & 0x0000ff0000000000ull) {
            dy = 5;
        } else if (tile & 0x000000ff00000000ull) {
            dy = 4;
        } else if (tile & 0x00000000ff000000ull) {
            dy = 3;
        } else if (tile & 0x0000000000ff0000ull) {
            dy = 2;
        } else if (tile & 0x000000000000ff00ull) {
            dy = 1;
        }
        return dy;
    }

    uint64_t uint64_tl(uint64_t tile) {
        uint64_t dz = 14;
        if (tile & 0x0000000000000001ull) {
            dz = 0;
        } else if (tile & 0x0000000000000102ull) {
            dz = 1;
        } else if (tile & 0x0000000000010204ull) {
            dz = 2;
        } else if (tile & 0x0000000001020408ull) {
            dz = 3;
        } else if (tile & 0x0000000102040810ull) {
            dz = 4;
        } else if (tile & 0x0000010204081020ull) {
            dz = 5;
        } else if (tile & 0x0001020408102040ull) {
            dz = 6;
        } else if (tile & 0x0102040810204080ull) {
            dz = 7;
        } else if (tile & 0x0204081020408000ull) {
            dz = 8;
        } else if (tile & 0x0408102040800000ull) {
            dz = 9;
        } else if (tile & 0x0810204080000000ull) {
            dz = 10;
        } else if (tile & 0x1020408000000000ull) {
            dz = 11;
        } else if (tile & 0x2040800000000000ull) {
            dz = 12;
        } else if (tile & 0x4080000000000000ull) {
            dz = 13;
        }
        return dz;
    }

    uint64_t uint64_top(uint64_t tile) {
        uint64_t dy = 7;
        if (tile & 0x00000000000000ffull) {
            dy = 0;
        } else if (tile & 0x000000000000ff00ull) {
            dy = 1;
        } else if (tile & 0x0000000000ff0000ull) {
            dy = 2;
        } else if (tile & 0x00000000ff000000ull) {
            dy = 3;
        } else if (tile & 0x000000ff00000000ull) {
            dy = 4;
        } else if (tile & 0x0000ff0000000000ull) {
            dy = 5;
        } else if (tile & 0x00ff000000000000ull) {
            dy = 6;
        }
        return dy;
    }

    uint64_t uint64_right(uint64_t tile) {
        uint64_t dx = 0;
        if (tile & 0x8080808080808080ull) {
            dx = 7;
        } else if (tile & 0x4040404040404040ull) {
            dx = 6;
        } else if (tile & 0x2020202020202020ull) {
            dx = 5;
        } else if (tile & 0x1010101010101010ull) {
            dx = 4;
        } else if (tile & 0x0808080808080808ull) {
            dx = 3;
        } else if (tile & 0x0404040404040404ull) {
            dx = 2;
        } else if (tile & 0x0202020202020202ull) {
            dx = 1;
        }
        return dx;
    }

    uint64_t uint64_left(uint64_t tile) {
        uint64_t dx = 7;
        if (tile & 0x0101010101010101ull) {
            dx = 0;
        } else if (tile & 0x0202020202020202ull) {
            dx = 1;
        } else if (tile & 0x0404040404040404ull) {
            dx = 2;
        } else if (tile & 0x0808080808080808ull) {
            dx = 3;
        } else if (tile & 0x1010101010101010ull) {
            dx = 4;
        } else if (tile & 0x2020202020202020ull) {
            dx = 5;
        } else if (tile & 0x4040404040404040ull) {
            dx = 6;
        }
        return dx;
    }

}
