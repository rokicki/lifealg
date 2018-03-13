#pragma once

#include "upattern.h"

namespace apg {

    template<int W, int H>
    struct Incube {
        uint64_t d[H];
        uint64_t hist[H];
    };

    template<int W, int H>
    class incubator {

        public:
        std::map<std::pair<int, int>, Incube<W, H> > tiles;

        int isGlider(Incube<W, H> *sqt, int px, int py, bool nuke, uint64_t* cachearray) {

            if (cachearray[py] & (1ull << px)) { return 2; }

            if ((px < 2) || (py < 2) || (px > W - 3) || (py > H - 5)) { return 0; }

            int x = px;
            int y = py + 1;

            if ((sqt->d[y-2] | sqt->d[y+2]) & (31ull << (x - 2))) { return 0; }

            uint64_t projection = ((sqt->d[y+1] | sqt->d[y] | sqt->d[y-1]) >> (x - 2)) & 31ull;

            if (projection == 7) {
                x -= 1; // ..ooo
            } else if (projection == 14) {
                // .ooo.
            } else if (projection == 28) {
                x += 1; // ooo..
            } else {
                // The shadow does not match that of a glider.
                return 0;
            }

            // Now (x, y) should be the central cell of the putative glider.

            if ((x < 3) || (x > (W - 4))) {
                return 0;
            } else if ((sqt->d[y] | sqt->d[y-1] | sqt->d[y+1]) & (99ull << (x - 3))) {
                return 0;
            } else if ((sqt->d[y-2] | sqt->d[y+2]) & (127ull << (x - 3))) {
                return 0;
            } else if ((sqt->d[y-3] | sqt->d[y+3]) & (60ull << (x - 3))) {
                return 0;
            } else {
                // 512 bits to indicate which 16 of the 512 3-by-3 bitpatterns correspond
                // to a glider in some orientation and phase.
                unsigned long long array [] = {
                    0x0000000000000000ull,
                    0x0400000000800000ull,
                    0x0000000000000000ull,
                    0x0010044000200000ull,
                    0x0400000000800000ull,
                    0x0010004002000800ull,
                    0x0000040002200800ull,
                    0x0000000000000000ull};

                int high3 = ((sqt->d[y]) >> (x - 1)) & 7ull;
                int low6 = (((sqt->d[y-1]) >> (x - 1)) & 7ull) | ((((sqt->d[y+1]) >> (x - 1)) & 7ull) << 3);

                if (array[high3] & (1ull << low6)) {

                    cachearray[y-1] |= (7ull << (x - 1));
                    cachearray[y  ] |= (7ull << (x - 1));
                    cachearray[y+1] |= (7ull << (x - 1));

                    if (nuke) {
                        // Destroy the glider:
                        for (int j = -1; j <= 1; j++) {
                            sqt->d[y+j] &= (~(7ull << (x-1)));
                        }
                    }

                    return 1;
                } else {
                    return 0;
                }
            }
        }

        int isBlinker(Incube<W, H>* sqt, int x, int y) {
            if ((x < 3) || (y < 3) || (x > W - 4) || (y > H - 4)) {
                return 0;
            } else if ((sqt->d[y  ] | sqt->hist[y  ]) & (99ull << (x - 3))) {
                return 0;
            } else if ((sqt->d[y-1] | sqt->hist[y-1] | sqt->d[y+1] | sqt->hist[y+1]) & (54ull << (x - 3))) {
                return 0;
            } else if ((sqt->d[y-2] | sqt->hist[y-2] | sqt->d[y+2] | sqt->hist[y+2]) & (62ull << (x - 3))) {
                return 0;
            } else if ((sqt->d[y-3] | sqt->hist[y-3] | sqt->d[y+3] | sqt->hist[y+3]) & ( 8ull << (x - 3))) {
                return 0;
            } else {
                // std::cout << "Blinker detected." << std::endl;
                sqt->d[y] &= (~(7ull << (x-1)));
                sqt->d[y-1] &= (~(1ull << x));
                sqt->d[y+1] &= (~(1ull << x));
                return 3;
            }
        }

        int isVerticalBeehive(Incube<W, H>* sqt, int x, int y) {
            if ((x < 3) || (y < 2) || (x > W - 4) || (y > H - 6)) {
                return 0;
            } else if ((sqt->d[y+1] | sqt->hist[y+1] | sqt->d[y+2] | sqt->hist[y+2]) & (107ull << (x - 3))) {
                return 0;
            } else if ((sqt->d[y  ] | sqt->hist[y  ] | sqt->d[y+3] | sqt->hist[y+3]) & (119ull << (x - 3))) {
                return 0;
            } else if ((sqt->d[y-1] | sqt->hist[y-1] | sqt->d[y+4] | sqt->hist[y+4]) & ( 62ull << (x - 3))) {
                return 0;
            } else if ((sqt->d[y-2] | sqt->hist[y-2] | sqt->d[y+5] | sqt->hist[y+5]) & (  8ull << (x - 3))) {
                return 0;
            } else {
                sqt->d[y  ] &= (~(1ull << x));
                sqt->d[y+1] &= (~(5ull << (x-1)));
                sqt->d[y+2] &= (~(5ull << (x-1)));
                sqt->d[y+3] &= (~(1ull << x));
                return 6;
            }
        }

        int isBlock(Incube<W, H>* sqt, int x, int y) {
            if ((sqt->d[y] | sqt->hist[y] | sqt->d[y+1] | sqt->hist[y+1]) & (51ull << (x - 2))) {
                return 0;
            } else if ((sqt->d[y-1] | sqt->hist[y-1] | sqt->d[y+2] | sqt->hist[y+2]) & (63ull << (x - 2))) {
                return 0;
            } else if ((sqt->d[y-2] | sqt->hist[y-2] | sqt->d[y+3] | sqt->hist[y+3]) & (30ull << (x - 2))) {
                return 0;
            } else {
                sqt->d[y  ] &= (~(3ull << x));
                sqt->d[y+1] &= (~(3ull << x));
                return 4;
            }
        }

        int isAnnoyance(Incube<W, H>* sqt, int x, int y) {

            if ((x < 2) || (y < 2) || (x > W - 4) || (y > H - 4)) { return 0; }

            if ((sqt->d[y] >> (x + 1)) & 1) {
                if ((sqt->d[y+1] >> x) & 1) {
                    return isBlock(sqt, x, y);
                } else {
                    return isBlinker(sqt, x+1, y);
                }
            } else {
                if ((sqt->d[y+1] >> x) & 1) {
                    return isBlinker(sqt, x, y+1);
                } else {
                    return isVerticalBeehive(sqt, x, y);
                }
            }
        }

        void purge(Incube<W, H>* sqt, uint64_t* excess) {
            for (int y = 0; y < H; y++) {
                uint64_t r = sqt->d[y];
                while (r != 0) {
                    uint64_t x = __builtin_ctzll(r);
                    int annoyance = isAnnoyance(sqt, x, y);
                    r ^= (1ull << x);
                    r &= sqt->d[y];
                    if ((annoyance > 0) && (excess != 0)) {
                        excess[annoyance] += 1;
                    }
                }
            }
        }

        void purge(uint64_t* excess) {
            for (auto it = tiles.begin(); it != tiles.end(); ++it) {
                purge(&(it->second), excess);
            }
        }

        void to_bitworld(bitworld &bw, int z) {
            for (auto it = tiles.begin(); it != tiles.end(); ++it) {
                Incube<W, H>* sqt = &(it->second);
                int64_t x = it->first.first * (W / 8);
                int64_t y = it->first.second * (H / 8);
                uint64_t* q = (z ? sqt->hist : sqt->d);
                uint64_t f[8] = {0};
                for (uint64_t j = 0; j < (H / 8); j++) {
                    int bis = best_instruction_set();
                    if (bis >= 9) {
                        transpose_bytes_avx(q + (8*j), f);
                    } else {
                        transpose_bytes_sse2(q + (8*j), f);
                    }
                    for (uint64_t i = 0; i < (W / 8); i++) {
                        if (f[i]) { bw.world.emplace(std::pair<int32_t, int32_t>(x + i, y + j), f[i]); }
                    }
                }
            }
        }
    };

    void copycells(upattern<VTile28, 28>* curralgo, incubator<56, 56>* destalgo) {

        for (auto it = curralgo->tiles.begin(); it != curralgo->tiles.end(); ++it) {
            VTile28* sqt = &(it->second);
            int64_t tx = (sqt->coords & 0xffffffffu) - 0x80000000u;
            int64_t tw = (sqt->coords >> 32) - 0x80000000u;

            bool split = (((2 * tx - tw) & 3) == 3);

            for (int half = 0; half < 1 + split; half++) {

                int mx = 2 * tx - tw + half;
                int my = -tw;

                int lx = mx % 4;
                int ly = my % 2;

                if (lx < 0) {lx += 4;}
                if (ly < 0) {ly += 2;}

                int tx = (mx - lx) / 4;
                int ty = (my - ly) / 2;

                Incube<56, 56>* sqt2 = &(destalgo->tiles[std::pair<int, int>(tx, ty)]);

                for (int i = 0; i < 28; i++) {
                    uint64_t insert;
                    if (half == 1) {
                        insert = (sqt->d[i+2] >> 16) & 0x3fff;
                    } else {
                        insert = (sqt->d[i+2] >> 2) & (split ? 0x3fff : 0xfffffff);
                    }
                    sqt2->d[i + 28 * ly] |= (insert << (14 * lx));
                    if (half == 1) {
                        insert = (sqt->hist[i+2] >> 16) & 0x3fff;
                    } else {
                        insert = (sqt->hist[i+2] >> 2) & (split ? 0x3fff : 0xfffffff);
                    }
                    sqt2->hist[i + 28 * ly] |= (insert << (14 * lx));
                }
            }
        }
    }
}
