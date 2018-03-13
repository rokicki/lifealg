#pragma once

#define LIFELIB_VERSION "ll1.23"

#include "bitbounds.h"
#include <stdint.h>
#include <map>
#include <utility>
#include <iostream>

namespace apg {

    // Convert (x, y) pairs into Morton order:
    uint64_t morton32(uint64_t x, uint64_t y) {
        uint64_t z = x | (y << 32);
        z = (z & 0xffff00000000ffffull) | ((z & 0x00000000ffff0000ull) << 16) | ((z & 0x0000ffff00000000ull) >> 16);
        z = (z & 0xff0000ffff0000ffull) | ((z & 0x0000ff000000ff00ull) << 8) | ((z & 0x00ff000000ff0000ull) >> 8);
        z = (z & 0xf00ff00ff00ff00full) | ((z & 0x00f000f000f000f0ull) << 4) | ((z & 0x0f000f000f000f00ull) >> 4);
        z = (z & 0xc3c3c3c3c3c3c3c3ull) | ((z & 0x0c0c0c0c0c0c0c0cull) << 2) | ((z & 0x3030303030303030ull) >> 2);
        z = (z & 0x9999999999999999ull) | ((z & 0x2222222222222222ull) << 1) | ((z & 0x4444444444444444ull) >> 1);
        return z;
    }

    uint64_t inflcorner(uint64_t x) {
        uint64_t z = x & (0x000000000f0f0f0full);
        z = (z & 0xffff00000000ffffull) | ((z & 0x00000000ffff0000ull) << 16) | ((z & 0x0000ffff00000000ull) >> 16);
        z = (z & 0xff0000ffff0000ffull) | ((z & 0x0000ff000000ff00ull) << 8) | ((z & 0x00ff000000ff0000ull) >> 8);
        z = (z & 0xc3c3c3c3c3c3c3c3ull) | ((z & 0x0c0c0c0c0c0c0c0cull) << 2) | ((z & 0x3030303030303030ull) >> 2);
        z = (z & 0x9999999999999999ull) | ((z & 0x2222222222222222ull) << 1) | ((z & 0x4444444444444444ull) >> 1);
        z |= (z << 1);
        z |= (z << 8);
        return z;
    }

    std::pair<uint64_t, uint64_t> morton64(uint64_t x, uint64_t y) {
        uint64_t zhigh = morton32(x >> 32, y >> 32);
        uint64_t zlow = morton32(x & 0xffffffffull, y & 0xffffffffull);
        return std::make_pair(zhigh, zlow);
    }

    // Compare apgcode representations to determine which one is simpler:
    std::string comprep(std::string a, std::string b)
    {
        if (a.compare("#") == 0) {
            return b;
        } else if (b.compare("#") == 0) {
            return a;
        } else if (a.length() < b.length()) {
            return a;
        } else if (b.length() < a.length()) {
            return b;
        } else if (a.compare(b) < 0) {
            return a;
        } else {
            return b;
        }
    }

    /*
     * A container for storing two-dimensional binary data as a collection of
     * 8-by-8 tiles. Limited to universes of size (2 ** 35)-by-(2 ** 35).
     */
    struct bitworld {

        public:
        std::map<std::pair<int32_t, int32_t>, uint64_t> world;

        // Return a Morton-ordered associative array containing the bitworld,
        // suitable for converting into a compressed quadtree representation.
        void mortonmap(std::map<uint64_t, uint64_t> *mmap) const {
            std::map<std::pair<int32_t, int32_t>, uint64_t>::const_iterator it;
            for (it = world.cbegin(); it != world.cend(); ++it ) {
                uint32_t x = 0x80000000u + ((uint32_t) it->first.first);
                uint32_t y = 0x80000000u + ((uint32_t) it->first.second);
                if (it->second) { mmap->emplace(morton32(x, y), it->second); }
            }
        }

        bool getcell(uint64_t x, uint64_t y) {
            int32_t u = (uint32_t) (x >> 3);
            int32_t v = (uint32_t) (y >> 3);
            uint64_t w = (world[std::make_pair(u, v)] >> ((x & 7) | ((y & 7) << 3)));
            return (w & 1);
        }

        void setcell(uint64_t x, uint64_t y, bool newval) {
            int32_t u = (uint32_t) (x >> 3);
            int32_t v = (uint32_t) (y >> 3);
            uint64_t w = (1ull << ((x & 7) | ((y & 7) << 3)));
            if (newval) {
                world[std::make_pair(u, v)] |= w;
            } else {
                world[std::make_pair(u, v)] &= (~w);
            }
        }

        std::string canonise_orientation(int length, int breadth, int ox, int oy, int a, int b, int c, int d)
        {
            std::string representation;

            char charnames[] = "0123456789abcdefghijklmnopqrstuvwxyz";

            for (int v = 0; v < ((breadth-1)/5)+1; v++) {
                int zeroes = 0;
                if (v != 0) { representation += 'z'; }
                for (int u = 0; u < length; u++) {
                    int baudot = 0;
                    for (int w = 0; w < 5; w++) {
                        int64_t x = ox + a*u + b*(5*v + w);
                        int64_t y = oy + c*u + d*(5*v + w);
                        baudot = (baudot >> 1) + 16 * getcell(x, y);
                    }
                    if (baudot == 0) {
                        zeroes += 1;
                    } else {
                        while (zeroes >= 40) {
                            representation += "yz";
                            zeroes -= 39;
                        }
                        if (zeroes > 0) {
                            if (zeroes == 1) {
                                representation += '0';
                            } else if (zeroes == 2) {
                                representation += 'w';
                            } else if (zeroes == 3) {
                                representation += 'x';
                            } else {
                                representation += 'y';
                                representation += charnames[zeroes - 4];
                            }
                        }
                        zeroes = 0;
                        representation += charnames[baudot];
                    }
                }
            }
            while ((representation.size() > 0) && (representation[representation.size() - 1] == 'z')) {
                representation = representation.substr(0, representation.size() - 1);
            }
            if (representation.size() == 0) { representation = "0"; }
            return representation;
        }

        bitworld& operator+=(const bitworld& rhs) {

            std::map<std::pair<int32_t, int32_t>, uint64_t>::const_iterator it;
            for (it = rhs.world.begin(); it != rhs.world.end(); ++it) {
                world[it->first] |= (it->second);
            }
            return *this;
        }

        bitworld& operator^=(const bitworld& rhs) {

            std::map<std::pair<int32_t, int32_t>, uint64_t>::const_iterator it;
            for (it = rhs.world.begin(); it != rhs.world.end(); ++it) {
                world[it->first] ^= (it->second);
            }
            return *this;
        }

        void intersect(const bitworld& rhs, uint64_t xx) {
            std::map<std::pair<int32_t, int32_t>, uint64_t>::iterator it;
            for (it = world.begin(); it != world.end(); /* no increment */ ) {
                std::map<std::pair<int32_t, int32_t>, uint64_t>::const_iterator it2 = rhs.world.find(it->first);
                uint64_t tile = xx;
                if (it2 != rhs.world.end()) {
                    tile ^= it2->second;
                }
                tile &= it->second;

                if (tile == 0) {
                    world.erase(it++);
                } else {
                    it->second = tile;
                    it++;
                }
            }
        }

        bitworld& operator-=(const bitworld& rhs) {

            if (rhs.world.size() < world.size()) {
                for (auto it = rhs.world.begin(); it != rhs.world.end(); ++it) {
                    world[it->first] &= (~(it->second));
                }
            } else {
                intersect(rhs, 0xffffffffffffffffull);
            }
            return *this;
        }

        bitworld& operator&=(const bitworld& rhs) {

            intersect(rhs, 0);
            return *this;

        }

        std::vector<std::pair<int64_t, int64_t> > getcells() {
            std::vector<std::pair<int64_t, int64_t> > celllist;
            std::map<std::pair<int32_t, int32_t>, uint64_t>::iterator it;
            for (it = world.begin(); it != world.end(); ++it ) {
                int64_t x = it->first.first;
                int64_t y = it->first.second;
                uint64_t tile = it->second;
                for (int i = 0; i < 64; i++) {
                    if ((tile >> i) & 1) {
                        celllist.push_back(std::pair<int64_t, int64_t>((x << 3) + (i & 7), (y << 3) + (i >> 3)));
                    }
                }
            }
            return celllist;
        }

        void printrepr() {
            std::map<std::pair<int32_t, int32_t>, uint64_t>::iterator it;
            for (it = world.begin(); it != world.end(); ++it ) {
                int32_t x = it->first.first;
                int32_t y = it->first.second;
                std::cout << '(' << x << ',' << y << ')' << std::endl;
                uint64_t tile = it->second;
                for (int i = 0; i < 64; i++) {
                    std::cout << ((tile & 1) ? '*' : '.');
                    tile = tile >> 1;
                    if (i % 8 == 7) { std::cout << std::endl; }
                }
            }
        }

        uint64_t population() {

            uint64_t pop = 0;
            std::map<std::pair<int32_t, int32_t>, uint64_t>::iterator it;
            for (it = world.begin(); it != world.end(); /* no increment */ ) {
                uint64_t tile = it->second;
                if (tile == 0) {
                    // Empty tile; erase:
                    world.erase(it++);
                } else {
                    pop += __builtin_popcountll(tile);
                    it++;
                }
            }
            return pop;

        }

        int64_t get_tldiag() {

            int64_t tldiag = 20000000000000ll;

            std::map<std::pair<int32_t, int32_t>, uint64_t>::iterator it;
            for (it = world.begin(); it != world.end(); /* no increment */ ) {
                uint64_t tile = it->second;
                if (tile == 0) {
                    world.erase(it++);
                } else {
                    int64_t diagonal = ((int64_t) it->first.first ) * 8;
                    diagonal += ((int64_t) it->first.second ) * 8;
                    if (diagonal < tldiag) {
                        int64_t dz = uint64_tl(tile);
                        if (diagonal + dz < tldiag) { tldiag = diagonal + dz; }
                    }
                    it++;
                }
            }

            return tldiag;
        }

        bool getbbox(int64_t *bbox) {

            // These are set outside the admissible range:
            int64_t left =    20000000000000ll;
            int64_t right =  -20000000000000ll;
            int64_t top =     20000000000000ll;
            int64_t bottom = -20000000000000ll;

            bool nonempty = false;

            std::map<std::pair<int32_t, int32_t>, uint64_t>::iterator it;
            for (it = world.begin(); it != world.end(); /* no increment */ ) {
                uint64_t tile = it->second;
                if (tile == 0) {
                    // Empty tile; erase:
                    world.erase(it++);
                } else {
                    // Only determine bounding box if bbox is a valid pointer:
                    if (bbox != 0) {
                        int64_t abscissa = ((int64_t) it->first.first ) * 8;
                        int64_t ordinate = ((int64_t) it->first.second) * 8;

                        if (abscissa < left) {
                            int64_t dx = uint64_left(tile);
                            if (abscissa + dx < left) { left = abscissa + dx; }
                        }

                        if (abscissa + 7 > right) {
                            int64_t dx = uint64_right(tile);
                            if (abscissa + dx > right) { right = abscissa + dx; }
                        }

                        if (ordinate < top) {
                            int64_t dy = uint64_top(tile);
                            if (ordinate + dy < top) { top = ordinate + dy; }
                        }

                        if (ordinate + 7 > bottom) {
                            int64_t dy = uint64_bottom(tile);
                            if (ordinate + dy > bottom) { bottom = ordinate + dy; }
                        }
                    }

                    // We've encountered a non-empty tile; proceed:
                    nonempty = true;
                    it++;
                }
            }

            if (nonempty && (bbox != 0)) {
                bbox[0] = left;
                bbox[1] = top;
                bbox[2] = (1 + right - left);
                bbox[3] = (1 + bottom - top);
            }

            return nonempty;
        }

        bitworld& clean() {
            std::map<std::pair<int32_t, int32_t>, uint64_t>::iterator it;
            for (it = world.begin(); it != world.end(); /* no increment */ ) {
                uint64_t tile = it->second;
                if (tile == 0) {
                    // Empty tile; erase:
                    world.erase(it++);
                } else {
                    it++;
                }
            }
            return *this;
        }

        bitworld inflate() {
            bitworld nb;
            for (auto it = world.begin(); it != world.end(); ++it) {
                uint64_t tile = it->second;
                std::pair<int32_t, int32_t> coords(it->first.first * 2, it->first.second * 2);
                if (tile != 0) {
                    nb.world[std::pair<int32_t, int32_t>(coords.first, coords.second)] = inflcorner(tile);
                    nb.world[std::pair<int32_t, int32_t>(coords.first+1, coords.second)] = inflcorner(tile >> 4);
                    nb.world[std::pair<int32_t, int32_t>(coords.first, coords.second+1)] = inflcorner(tile >> 32);
                    nb.world[std::pair<int32_t, int32_t>(coords.first+1, coords.second+1)] = inflcorner(tile >> 36);
                }
            }
            return nb;
        }

        bitworld get1cell() {

            bitworld x;
            std::map<std::pair<int32_t, int32_t>, uint64_t>::iterator it;
            for (it = world.begin(); it != world.end(); /* no increment */ ) {
                uint64_t tile = it->second;
                if (tile == 0) {
                    world.erase(it++);
                } else {
                    x.world[it->first] = tile & (~(tile - 1));
                    break;
                }
            }
            return x;

        }

        bitworld br1cell() {

            std::pair<int32_t, int32_t> record(-1000000000, -1000000000);
            uint64_t recordcell = 0;
            std::map<std::pair<int32_t, int32_t>, uint64_t>::iterator it;
            for (it = world.begin(); it != world.end(); /* no increment */ ) {
                uint64_t tile = it->second;
                if (tile == 0) {
                    world.erase(it++);
                } else {
                    if (it->first.first + it->first.second > record.first + record.second) {
                        record = it->first;
                        recordcell = tile & (~(tile - 1));
                    }
                    it++;
                }
            }
            bitworld x;
            x.world[record] = recordcell;
            return x;
        }

    };

    std::vector<bitworld> rle2vec(std::string rle) {
        std::vector<bitworld> planes;
        uint64_t x = 0; uint64_t y = 0; uint64_t count = 0;
        uint64_t colour = 0;
        for (unsigned int i = 0; i < rle.size(); i++) {
            char c = rle[i];
            if ((c >= '0') && (c <= '9')) {
                count *= 10;
                count += (c - '0');
            } else if ((c == 'b') || (c == '.')) {
                if (count == 0) { count = 1; }
                x += count; count = 0;
            } else if (c == '$') {
                if (count == 0) { count = 1; }
                y += count; x = 0; count = 0;
            } else if ((c == 'o') || ((c >= 'A') && (c <= 'X'))) {
                if (count == 0) { count = 1; }
                if (c == 'o') {
                    colour = colour * 24 + 1;
                } else {
                    colour = colour * 24 + (c - 'A') + 1;
                }
                uint64_t bp = 0;
                while (colour > 0) {
                    if (colour & 1) {
                        if (bp >= planes.size()) { planes.resize(bp+1); }
                        for (uint64_t j = 0; j < count; j++) {
                            planes[bp].setcell(x+j, y, 1);
                        }
                    }
                    bp += 1; colour = colour >> 1;
                }
                x += count; count = 0; colour = 0;
            } else if ((c >= 'p') && (c <= 'z')) {
                uint64_t m = c - 'o';
                colour = colour * 11 + (m % 11);
            } else if (c == '!') { break; }
        }
        return planes;
    }

    bitworld _shift_left(const bitworld &a, uint32_t k) {
        bitworld b;
        std::map<std::pair<int32_t, int32_t>, uint64_t>::const_iterator it;
        uint64_t bitmask = (0x0101010101010101ull << k) - 0x0101010101010101ull;
        for (it = a.world.begin(); it != a.world.end(); ++it) {
            b.world[it->first] |= (it->second & (~bitmask)) >> k;
            int32_t newx = it->first.first - 1;
            int32_t newy = it->first.second;
            b.world[std::make_pair(newx, newy)] |= (it->second & bitmask) << (8-k);
        }
        return b.clean();
    }

    bitworld _shift_right(const bitworld &a, uint32_t k) {
        bitworld b;
        std::map<std::pair<int32_t, int32_t>, uint64_t>::const_iterator it;
        uint64_t bitmask = (0x0101010101010101ull << (8-k)) - 0x0101010101010101ull;
        for (it = a.world.begin(); it != a.world.end(); ++it) {
            b.world[it->first] |= (it->second & bitmask) << k;
            int32_t newx = it->first.first + 1;
            int32_t newy = it->first.second;
            b.world[std::make_pair(newx, newy)] |= (it->second & (~bitmask)) >> (8-k);
        }
        return b.clean();
    }

    bitworld _shift_down(const bitworld &a, uint32_t k) {
        bitworld b;
        std::map<std::pair<int32_t, int32_t>, uint64_t>::const_iterator it;
        for (it = a.world.begin(); it != a.world.end(); ++it) {
            b.world[it->first] |= (it->second << (8*k));
            int32_t newx = it->first.first;
            int32_t newy = it->first.second + 1;
            b.world[std::make_pair(newx, newy)] |= (it->second >> (8*(8-k)));
        }
        return b.clean();
    }

    bitworld _shift_up(const bitworld &a, uint32_t k) {
        bitworld b;
        std::map<std::pair<int32_t, int32_t>, uint64_t>::const_iterator it;
        for (it = a.world.begin(); it != a.world.end(); ++it) {
            b.world[it->first] |= (it->second >> (8*k));
            int32_t newx = it->first.first;
            int32_t newy = it->first.second - 1;
            b.world[std::make_pair(newx, newy)] |= (it->second << (8*(8-k)));
        }
        return b.clean();
    }

    bitworld bleed(const bitworld &a, std::string s) {
        bitworld b = a;
        for (uint64_t i = 0; i < s.length(); i++) {
            char c = s[i];
            if (c == '9') {
                b += _shift_left(b, 1);
                b += _shift_right(b, 1);
                b += _shift_up(b, 1);
                b += _shift_down(b, 1);
            } else if (c == '5') {
                bitworld d = _shift_left(b, 1);
                d += _shift_right(b, 1);
                d += _shift_up(b, 1);
                d += _shift_down(b, 1);
                b += d;
            }
        }
        return b;
    }

    bitworld shift_bitworld(const bitworld &a, int64_t sx, int64_t sy) {
        uint32_t sl = ((uint32_t) (-sx)) & 7;
        uint32_t su = ((uint32_t) (-sy)) & 7;

        int32_t dx = (((int32_t) sl) + sx) >> 3;
        int32_t dy = (((int32_t) su) + sy) >> 3;

        bitworld b;
        std::map<std::pair<int32_t, int32_t>, uint64_t>::const_iterator it;
        for (it = a.world.begin(); it != a.world.end(); ++it) {
            int32_t newx = it->first.first + dx;
            int32_t newy = it->first.second + dy;
            if (it->second) { b.world[std::make_pair(newx, newy)] = it->second; }
        }
        if (sl) {b = _shift_left(b, sl); }
        if (su) {b = _shift_up(b, su); }
        return b;
    }

    bitworld fix_topleft(bitworld &a) {
        int64_t bbox[4] = {0};
        bool nonempty = a.getbbox(bbox);
        if (nonempty == false) { return a; }
        return shift_bitworld(a, -bbox[0], -bbox[1]);
    }

    bitworld grow_cluster(const bitworld &seed, const bitworld &backdrop, std::string growth) {

        bitworld agglom = seed;
        bitworld cluster = seed;

        while (cluster.population() != 0) {
            cluster = bleed(cluster, growth);
            cluster &= backdrop;
            cluster -= agglom;
            agglom += cluster;
        }
        return agglom;
    }

    std::string canonise_orientation(std::vector<bitworld> &bwv, int length, int breadth, int ox, int oy, int a, int b, int c, int d) {
        std::string s = "";
        for (uint64_t i = 0; i < bwv.size(); i++) {
            if (i != 0) { s += "_"; }
            s += bwv[i].canonise_orientation(length, breadth, ox, oy, a, b, c, d);
        }
        return s;
    }

    std::string wechslerise(std::vector<bitworld> &bwv, int64_t *rect) {

        std::string rep = "#";
        int64_t rect4 = rect[0] + rect[2] - 1;
        int64_t rect5 = rect[1] + rect[3] - 1;
        // if (((rect[2] + 2) * (rect[3] + 2)) <= 2000) {
            rep = comprep(rep, canonise_orientation(bwv, rect[2], rect[3], rect[0], rect[1],  1,  0,  0,  1));
            rep = comprep(rep, canonise_orientation(bwv, rect[2], rect[3], rect4, rect[1], -1,  0,  0,  1));
            rep = comprep(rep, canonise_orientation(bwv, rect[2], rect[3], rect[0], rect5,  1,  0,  0, -1));
            rep = comprep(rep, canonise_orientation(bwv, rect[2], rect[3], rect4, rect5, -1,  0,  0, -1));
            rep = comprep(rep, canonise_orientation(bwv, rect[3], rect[2], rect[0], rect[1],  0,  1,  1,  0));
            rep = comprep(rep, canonise_orientation(bwv, rect[3], rect[2], rect4, rect[1],  0, -1,  1,  0));
            rep = comprep(rep, canonise_orientation(bwv, rect[3], rect[2], rect[0], rect5,  0,  1, -1,  0));
            rep = comprep(rep, canonise_orientation(bwv, rect[3], rect[2], rect4, rect5,  0, -1, -1,  0));
        // }
        return rep;
    }

}
