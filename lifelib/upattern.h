#pragma once

#include "avxlife/uli.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include "bitworld.h"

namespace apg {

    const uint64_t udirections[] = {0x0000000000000001ull,
                                    0x0000000100000001ull,
                                    0x0000000100000000ull,
                                    0xffffffffffffffffull,
                                    0xfffffffeffffffffull,
                                    0xffffffff00000000ull};


    // Tile typename and width:
    template<typename T, int W>
    class upattern {
        /*
        * A container capable of running patterns in either (unhashed) ulife
        * or vlife. Appropriate for highly chaotic patterns such as random
        * soups, where there is no regularity other than period-2 detritus.
        * Otherwise, apg::pattern is a more appropriate choice of container.
        *
        * The upattern can support either an unbounded plane or a rectangular
        * torus whose width and height are divisible, respectively, by W and
        * 2W. In the former case, new tiles are allocated as the pattern
        * expands; in the latter case, the entire universe is preallocated in
        * memory at creation time.
        */

        private:
        uint64_t torus_width;
        uint64_t torus_height;

        public:
        std::unordered_map<uint64_t, T> tiles;
        std::vector<T*> modified;
        std::vector<T*> temp_modified;

        uint64_t tilesProcessed;

        T* coords2ptr(int64_t x, int64_t w) {
            // Returns a pointer to tile x + omega*w.
            int64_t ix = x; int64_t iw = w;
            if (torus_width > 0) {
                // Rectangular torus implemented on a hexagonal grid:
                iw = ((iw % torus_height) + torus_height) % torus_height;
                ix -= (w - iw) / 2;
                ix = ((ix % torus_width) + torus_width) % torus_width;
            }
            uint64_t p = ((uint64_t) (ix + 0x80000000ull));
            p += (((uint64_t) (iw + 0x80000000ull)) << 32);
            T* sqt = &(tiles[p]);
            sqt->coords = p;
            return sqt;
        }

        upattern() {
            // Construct an unbounded plane universe:
            tilesProcessed = 0;
            torus_width = 0;
            torus_height = 0;
        }

        upattern(int width, int height) {
            // Construct a rectangular toroidal universe:
            tilesProcessed = 0;
            torus_width = width / W;
            torus_height = height / W;

            if ((width == 0) || (height == 0)) { return; }

            // Create the full connectivity digraph for the torus:
            for (int i = 0; i < width; i++) {
                for (int j = 0; j < height; j++) {
                    T* sqt = coords2ptr(i, j);
                    sqt->neighbours[0] = coords2ptr(i+1, j);
                    sqt->neighbours[1] = coords2ptr(i+1, j+1);
                    sqt->neighbours[2] = coords2ptr(i, j+1);
                    sqt->neighbours[3] = coords2ptr(i-1, j);
                    sqt->neighbours[4] = coords2ptr(i-1, j-1);
                    sqt->neighbours[5] = coords2ptr(i, j-1);
                }
            } 
        }

        T* getNeighbour(T* sqt, int i) {
            if (!(sqt->neighbours[i])) {
                uint64_t x = sqt->coords + udirections[i];
                sqt->neighbours[i] = &tiles[x];
                sqt->neighbours[i]->coords = x;
            }
            return sqt->neighbours[i];
        }

        void decache() {
            for (auto it = tiles.begin(); it != tiles.end(); ++it) {
                T* sqt = &(it->second);
                if (sqt->updateflags == 0) {
                    modified.push_back(sqt);
                    sqt->updateflags |= 64;
                }
            }
        }

        void updateNeighbour(T* sqt, int i) {
            T* n = getNeighbour(sqt, i);
            if (n->updateflags == 0) {
                modified.push_back(n);
            }
            n->updateflags |= (1 << ((i + 3) % 6));
        }

        void updateBoundary(T* sqt) {
            if (sqt->updateflags & 1) {
                T* n = getNeighbour(sqt, 0);
                sqt->copyBoundary0(n);
            }
            if (sqt->updateflags & 2) {
                T* n = getNeighbour(sqt, 1);
                sqt->copyBoundary1(n);
            }
            if (sqt->updateflags & 4) {
                T* n = getNeighbour(sqt, 2);
                sqt->copyBoundary2(n);
            }
            if (sqt->updateflags & 8) {
                T* n = getNeighbour(sqt, 3);
                sqt->copyBoundary3(n);
            }
            if (sqt->updateflags & 16) {
                T* n = getNeighbour(sqt, 4);
                sqt->copyBoundary4(n);
            }
            if (sqt->updateflags & 32) {
                T* n = getNeighbour(sqt, 5);
                sqt->copyBoundary5(n);
            }
            sqt->updateflags = 0;
            temp_modified.push_back(sqt);
        }

        void runkgens(int rule, int family, uint64_t mantissa) {
            while (!modified.empty()) {
                updateBoundary(modified.back());
                modified.pop_back();
            }

            while (!temp_modified.empty()) {
                temp_modified.back()->updateTile(this, rule, family, mantissa);
                temp_modified.pop_back();
                tilesProcessed += 1;
            }
        }

        uint64_t valid_mantissa(int rule) {
            if (W == 16) {
                return uli_valid_mantissa(rule);
            } else {
                return 5;
            }
        }

        void advance(int rule, int history, uint64_t generations) {
            uint64_t grem = generations;
            int family = uli_get_family(rule) + history;
            uint64_t vmb = valid_mantissa(rule);
            while (grem > 0) {
                uint64_t m = (grem >= 8) ? 8 : grem;
                while (((vmb >> m) & 1) == 0) { m -= 1; }
                if (m == 0) { break; }
                runkgens(rule, family, m);
                grem -= m;
            }
        }

        int totalPopulation() {

            int population = 0;

            for (auto it = tiles.begin(); it != tiles.end(); ++it) {
                T* sqt = &(it->second);
                population += sqt->countPopulation();
            }

            return population;
        }

        bool nonempty() { return (totalPopulation() != 0); }

        void emplace_uint64(int z, int64_t x, int64_t y, uint64_t v) {
            int64_t ay = y;
            uint8_t dy = ((ay % W) + W) % W;
            ay -= dy;

            int64_t ax = ((int64_t) x) - (ay / 2);
            uint8_t dx = ((ax % W) + W) % W;
            ax -= dx;

            T* sqt = coords2ptr((ax / W), -(ay / W));

            sqt->eu64(this, z, dx, dy, v);
        }

        void insertPattern(std::vector<bitworld> &planes) {
            for (uint64_t i = 0; i < planes.size(); i++) {
                // if (i == N) { break; }
                for (auto it = planes[i].world.begin(); it != planes[i].world.end(); ++it) {
                    if (it->second != 0) {
                        emplace_uint64(i, 8 * it->first.first, 8 * it->first.second, it->second);
                    }
                }
            }
        }

        void clearHistory() {
            for (auto it = tiles.begin(); it != tiles.end(); ++it) {
                it->second.clearHistory();
            }
        }

        void extractPattern(std::vector<bitworld> &planes) {
            for (auto it = tiles.begin(); it != tiles.end(); ++it) {
                T* sqt = &(it->second);
                int64_t tx = (sqt->coords & 0xffffffffu) - 0x80000000u;
                int64_t tw = (sqt->coords >> 32) - 0x80000000u;
                int64_t x = tx * W - tw * (W/2);
                int64_t y = -tw * W;
                for (uint64_t i = 0; i < planes.size(); i++) {
                    if (sqt->nonempty(i)) {
                        planes[i] += shift_bitworld(sqt->to_bitworld(i), x, y);
                    }
                }
            }
        }

        void insertPattern(std::string s) {
            std::vector<bitworld> planes = rle2vec(s);
            insertPattern(planes);
        }

        uint64_t totalHash(int radius) {

            uint64_t globalhash = 0;

            for (auto it = tiles.begin(); it != tiles.end(); ++it)
            {
                T* sqt = &(it->second);
                int64_t tx = (sqt->coords & 0xffffffffu) - 0x80000000u;
                int64_t tw = (sqt->coords >> 32) - 0x80000000u;
                if (tx * tx + tw * tw - tx * tw < radius * radius) {
                    globalhash += sqt->hashTile() * (sqt->coords ^ 3141592653589793ull);
                }
            }

            return globalhash;
        }
    };

    struct VTile28 {

        uint32_t d[32]; // 128 bytes
        uint32_t hist[32]; // 128 bytes
        VTile28 *neighbours[6]; // 48 bytes
        uint64_t coords; // 8 bytes
        uint64_t hash; // 8 bytes
        int32_t population; // 4 bytes
        uint16_t updateflags; // 2 bytes
        bool populationCurrent; // 1 byte
        bool hashCurrent; // 1 byte

        bool nonempty(uint64_t z) {
            uint32_t* q = z ? hist : d;
            for (int i = 2; i < 30; i++) {
                if (q[i] & 0x3ffffffcu) { return true; }
            }
            return false;
        }

        void clearHistory() {
            std::memset(hist, 0, 128);
        }

        void copyBoundary1(VTile28 *n) {
            d[0] = ((n->d[28] & 0x3ffffffcu) << 14) | (d[0] & 0x0000ffffu);
            d[1] = ((n->d[29] & 0x3ffffffcu) << 14) | (d[1] & 0x0000ffffu);
        }

        void copyBoundary2(VTile28 *n) {
            d[0] = ((n->d[28] & 0x3ffffffcu) >> 14) | (d[0] & 0xffff0000u);
            d[1] = ((n->d[29] & 0x3ffffffcu) >> 14) | (d[1] & 0xffff0000u);
        }

        void copyBoundary3(VTile28 *n) {
            for (int i = 2; i < 30; i++) {
                d[i] = ((n->d[i] & 0x3ffffffcu) >> 28) | (d[i] & 0xfffffffcu);
            }
        }

        void copyBoundary4(VTile28 *n) {
            d[30] = ((n->d[2] & 0x3ffffffcu) >> 14) | (d[30] & 0xffff0000u);
            d[31] = ((n->d[3] & 0x3ffffffcu) >> 14) | (d[31] & 0xffff0000u);
        }

        void copyBoundary5(VTile28 *n) {
            d[30] = ((n->d[2] & 0x3ffffffcu) << 14) | (d[30] & 0x0000ffffu);
            d[31] = ((n->d[3] & 0x3ffffffcu) << 14) | (d[31] & 0x0000ffffu);
        }

        void copyBoundary0(VTile28 *n) {
            for (int i = 2; i < 30; i++) {
                d[i] = ((n->d[i] & 0x3ffffffcu) << 28) | (d[i] & 0x3fffffffu);
            }
        }

        void updateTile(upattern<VTile28, 28>* owner, int rule, int family, int mantissa) {

            (void) mantissa;
            uint32_t diffs[3] = {0};

            int r;
            if (family == 1) {
                r = iterate_var_32_28(rule, d, hist, diffs);
            } else {
                r = iterate_var_32_28(rule, d, diffs);
            }

            if ((r != 1) && (diffs[0] & 0x3ffffffcu)) {
                populationCurrent = false;
                hashCurrent = false;
                if (updateflags == 0) { owner->modified.push_back(this); }
                updateflags |= 64;
                if (diffs[0] & 0x30000000u) { owner->updateNeighbour(this, 0); }
                if (diffs[0] & 0x0000000cu) { owner->updateNeighbour(this, 3); }
                if (diffs[1] & 0x3fffc000u) { owner->updateNeighbour(this, 1); }
                if (diffs[1] & 0x0003fffcu) { owner->updateNeighbour(this, 2); }
                if (diffs[2] & 0x3fffc000u) { owner->updateNeighbour(this, 5); }
                if (diffs[2] & 0x0003fffcu) { owner->updateNeighbour(this, 4); }
            }
        }

        uint64_t hashTile() {
            if (hashCurrent) { return hash; }
            uint64_t partialhash = 0;

            for (int i = 2; i < 30; i++) {
                partialhash = partialhash * (partialhash +447840759955ull) + i * (d[i] & 0x3ffffffcu);
            }

            hash = partialhash;
            hashCurrent = true;
            return partialhash;
        }

        int countPopulation() {
            if (populationCurrent) { return population; }
            int pop = 0;
            uint64_t y[14];
            std::memcpy(y, d + 2, 112);

            for (int i = 0; i < 14; i++) {
                pop += __builtin_popcountll(y[i] & 0x3ffffffc3ffffffcull);
            }

            population = pop;
            populationCurrent = true;
            return pop;
        }

        bitworld to_bitworld(int z) {
            uint32_t* q = (z ? hist : d);
            bitworld bw;
            uint32_t e[32] = {0};
            for (uint64_t i = 0; i < 28; i++) {
                e[i] = (q[i + 2] >> 2) & 0xfffffff;
            }
            uint64_t f[4];
            for (uint64_t j = 0; j < 4; j++) {
                int bis = best_instruction_set();
                if (bis >= 9) {
                    twofifths_avx(e + (8*j), f);
                } else {
                    twofifths_sse2(e + (8*j), f);
                }
                for (uint64_t i = 0; i < 4; i++) {
                    if (f[i]) { bw.world.emplace(std::pair<int32_t, int32_t>(i, j), f[i]); }
                }
            }
            return bw;
        }

        void eu64(upattern<VTile28, 28>* owner, int z, uint8_t dx, uint8_t dy, uint64_t v) {

            // std::cout << ((int) dx) << " " << ((int) dy) << " " << v << std::endl;

            if ((v == 0) || (z >= 2)) { return; }

            for (int i = 0; i < 6; i++) { owner->updateNeighbour(this, i); }
            if (updateflags == 0) { owner->modified.push_back(this); }
            updateflags |= 64;

            uint32_t* q = (z ? hist : d);

            if (dy > 20) {
                owner->getNeighbour(this, 4)->eu64(owner, z, dx + 14, 0, v >> (8 * (28 - dy)));
            }
            if (dx >= 28) {
                owner->getNeighbour(this, 0)->eu64(owner, z, dx - 28, dy, v);
            } else if (dx > 20) {
                uint64_t bitmask = 0x0101010101010101ull;
                bitmask = (bitmask << (dx - 20)) - bitmask;
                owner->getNeighbour(this, 0)->eu64(owner, z, 0, dy, (v >> (28 - dx)) & bitmask);
            }

            if (dx < 28) {
                for (uint64_t i = 0; i < 8; i++) {
                    uint64_t newy = i + dy;
                    if (newy < 28) {
                        q[newy + 2] |= ((((v >> (8 * i)) & 255) << (dx + 2)) & 0x3ffffffcu);
                    }
                }
            }
        }

    };

    // N = total number of layers;
    // M = number of non-passive layers:
    template<int N, int M>
    struct UTile {

        uint64_t a[4 * N]; // 32N bytes
        uint64_t b[4 * N]; // 32N bytes
        uint64_t c[4 * N]; // 32N bytes
        uint64_t d[4 * N]; // 32N bytes
        UTile<N, M> *neighbours[6]; // 48 bytes

        // We store both coordinates in a single uint64_t, because why not?
        // Provided we don't get anywhere near 2^32 away from the origin,
        // the implementation details won't leak. This means we don't have
        // to provide a (slow) std::hash<int, int> implementation in order
        // to reap the benefits of an unordered_map; moreover, manipulating
        // a uint64_t should be faster than a pair of ints.
        uint64_t coords; // 8 bytes

        uint64_t hash; // 8 bytes

        int32_t population; // 4 bytes
        uint16_t updateflags; // 2 bytes
        bool populationCurrent; // 1 byte
        bool hashCurrent; // 1 byte

        // sizeof(UTile<N, M>) == 128N + 72 bytes (no alignment space wasted)

        void clearHistory() {
            std::memset(a + (4 * M), 0, 32 * (N - M));
            std::memset(b + (4 * M), 0, 32 * (N - M));
            std::memset(c + (4 * M), 0, 32 * (N - M));
            std::memset(d + (4 * M), 0, 32 * (N - M));
        }

        void copyBoundary0(UTile<N, M>* n) {
            for (int i = 0; i < M; i++) {
                b[3 + 4*i] = n->a[3 + 4*i];
                d[1 + 4*i] = n->c[1 + 4*i];
            }
        }
        void copyBoundary1(UTile<N, M>* n) {
            for (int i = 0; i < M; i++) {
                b[0 + 4*i] = n->c[1 + 4*i];
                b[1 + 4*i] = n->d[0 + 4*i];
            }
        }
        void copyBoundary2(UTile<N, M>* n) {
            for (int i = 0; i < M; i++) {
                a[0 + 4*i] = n->c[1 + 4*i];
                a[1 + 4*i] = n->d[0 + 4*i];
            }
        }
        void copyBoundary3(UTile<N, M>* n) {
            for (int i = 0; i < M; i++) {
                a[2 + 4*i] = n->b[2 + 4*i];
                c[0 + 4*i] = n->d[0 + 4*i];
            }
        }
        void copyBoundary4(UTile<N, M>* n) {
            for (int i = 0; i < M; i++) {
                c[2 + 4*i] = n->a[3 + 4*i];
                c[3 + 4*i] = n->b[2 + 4*i];
            }
        }
        void copyBoundary5(UTile<N, M>* n) {
            for (int i = 0; i < M; i++) {
                d[2 + 4*i] = n->a[3 + 4*i];
                d[3 + 4*i] = n->b[2 + 4*i];
            }
        }

        uint64_t hashTile() {

            if (hashCurrent) { return hash; }
            uint64_t partialhash = 0;

            for (int i = 0; i < M; i++) {
                partialhash = partialhash * (partialhash +447840759955ull);
                partialhash += (a[3 + 4*i] ^ (1093 * b[2 + 4*i]));
                partialhash += ((641 * d[4*i]) ^ (3511 * c[1 + 4*i]));
            }

            hash = partialhash;
            hashCurrent = true;
            return partialhash;
        }

        void updateTile(upattern<UTile<N, M>, 16>* owner, int rule, int family, uint64_t mantissa) {
            uint64_t outleafx[4*N] = {0ull};
            uint64_t* inleafxs[4] = {a, b, c, d};
            int r = universal_leaf_iterator<N>(rule, family, mantissa, inleafxs, outleafx);
            if (r != 1) {
                uint64_t diff[4] = {0ull};
                for (int i = 0; i < M; i++) {
                    diff[3] |= (outleafx[0 + 4*i] ^ a[3 + 4*i]);
                    diff[2] |= (outleafx[1 + 4*i] ^ b[2 + 4*i]);
                    diff[1] |= (outleafx[2 + 4*i] ^ c[1 + 4*i]);
                    diff[0] |= (outleafx[3 + 4*i] ^ d[0 + 4*i]);
                }
                if (diff[0] | diff[1] | diff[2] | diff[3]) {
                    populationCurrent = false;
                    hashCurrent = false;
                    if (updateflags == 0) { owner->modified.push_back(this); }
                    updateflags |= 64;
                }
                if (diff[0] | diff[2]) {
                    owner->updateNeighbour(this, 0);
                }
                if (diff[2] | diff[3]) {
                    owner->updateNeighbour(this, 1);
                    owner->updateNeighbour(this, 2);
                }
                if (diff[3] | diff[1]) {
                    owner->updateNeighbour(this, 3);
                }
                if (diff[1] | diff[0]) {
                    owner->updateNeighbour(this, 4);
                    owner->updateNeighbour(this, 5);
                }
            }
            for (int i = 0; i < N; i++) {
                a[3 + 4*i] = outleafx[0 + 4*i];
                b[2 + 4*i] = outleafx[1 + 4*i];
                c[1 + 4*i] = outleafx[2 + 4*i];
                d[0 + 4*i] = outleafx[3 + 4*i];
            }
        }

        bool nonempty(uint64_t i) {
            return (a[4*i+3] | b[4*i+2] | c[4*i+1] | d[4*i]);
        }

        int countPopulation() {
            if (populationCurrent) { return population; }
            int pop = 0;
            uint64_t diff[4] = {0ull};
            for (int i = 0; i < M; i++) {
                diff[3] |= a[3 + 4*i];
                diff[2] |= b[2 + 4*i];
                diff[1] |= c[1 + 4*i];
                diff[0] |= d[0 + 4*i];
            }
            for (int i = 0; i < 4; i++) {
                pop += __builtin_popcountll(diff[i]);
            }
            population = pop;
            populationCurrent = true;
            return pop;
        }

        bitworld to_bitworld(int z) {
            bitworld bw;
            if (a[4*z+3]) { bw.world.emplace(std::pair<int32_t, int32_t>(0, 0), a[4*z+3]); }
            if (b[4*z+2]) { bw.world.emplace(std::pair<int32_t, int32_t>(1, 0), b[4*z+2]); }
            if (c[4*z+1]) { bw.world.emplace(std::pair<int32_t, int32_t>(0, 1), c[4*z+1]); }
            if (d[4*z+0]) { bw.world.emplace(std::pair<int32_t, int32_t>(1, 1), d[4*z+0]); }
            return bw;
        }

        void eu64(upattern<UTile<N, M>, 16>* owner, int z, uint8_t dx, uint8_t dy, uint64_t v) {

            for (int i = 0; i < 6; i++) { owner->updateNeighbour(this, i); }
            if (updateflags == 0) { owner->modified.push_back(this); }
            updateflags |= 64;

            uint8_t dz = (dx / 8) + (dy / 8) * 2;

            if (dz == 0) { a[4*z+3] = v; }
            if (dz == 1) { b[4*z+2] = v; }
            if (dz == 2) { c[4*z+1] = v; }
            if (dz == 3) { d[4*z+0] = v; }

        }

    };

} // namespace apg

