#pragma once

#include "hashtrees/hypertree.h"
#include "bitworld.h"
#include <map>
#include <utility>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

namespace apg {

    uint64_t transform_uint64(uint64_t x, uint8_t perm) {

        uint64_t c[4];

        c[0] = x & 0x000000000f0f0f0full;
        c[1] = (x >> 4) & 0x000000000f0f0f0full;
        c[2] = (x >> 32) & 0x000000000f0f0f0full;
        c[3] = (x >> 36) & 0x000000000f0f0f0full;

        uint64_t y = c[perm & 3] | (c[(perm >> 2) & 3] << 4) | (c[(perm >> 4) & 3] << 32) | (c[(perm >> 6) & 3] << 36);

        c[0] = y & 0x0000333300003333ull;
        c[1] = (y >> 2) & 0x0000333300003333ull;
        c[2] = (y >> 16) & 0x0000333300003333ull;
        c[3] = (y >> 18) & 0x0000333300003333ull;

        y = c[perm & 3] | (c[(perm >> 2) & 3] << 2) | (c[(perm >> 4) & 3] << 16) | (c[(perm >> 6) & 3] << 18);

        c[0] = y & 0x0055005500550055ull;
        c[1] = (y >> 1) & 0x0055005500550055ull;
        c[2] = (y >> 8) & 0x0055005500550055ull;
        c[3] = (y >> 9) & 0x0055005500550055ull;

        y = c[perm & 3] | (c[(perm >> 2) & 3] << 1) | (c[(perm >> 4) & 3] << 8) | (c[(perm >> 6) & 3] << 9);

        return y;
    }

    template<typename I>
    struct lifemeta {

        I res;
        I aux;

    };

    template<typename I>
    class lifetree_abstract {

        public:

        uint64_t gc_threshold;

        virtual uint64_t newihandle(hypernode<I> hnode) = 0;
        virtual void delhandle(uint64_t ihandle) = 0;
        virtual void delhandle(std::string handle) = 0;
        virtual void sethandle(uint64_t ihandle, hypernode<I> hnode) = 0;
        virtual void sethandle(std::string handle, hypernode<I> hnode) = 0;
        virtual hypernode<I> gethandle(uint64_t ihandle) = 0;
        virtual hypernode<I> gethandle(std::string handle) = 0;
        virtual uint64_t counthandles() = 0;
        virtual uint64_t countlayers() = 0;
        virtual void force_gc() = 0;
        virtual bool threshold_gc(uint64_t threshold) = 0;
        virtual uint64_t getcell_recurse(hypernode<I> hnode, uint64_t x, uint64_t y) = 0;
        virtual void write_macrocell(std::ostream &outstream, hypernode<I> hnode, std::string rule) = 0;
        virtual hypernode<I> _string32(std::string s) = 0;
        virtual std::string _string32(hypernode<I> hnode) = 0;

        bool threshold_gc() {
            return threshold_gc(gc_threshold);
        }

        virtual kiventry<nicearray<I, 4>, I, lifemeta<I> >* ind2ptr_nonleaf(uint32_t depth, I index) = 0;
        virtual I make_nonleaf(uint32_t depth, nicearray<I, 4> contents) = 0;
        virtual hypernode<I> make_nonleaf_hn(uint32_t depth, nicearray<I, 4> contents) = 0;
        virtual I getpop_recurse(hypernode<I> hnode, I modprime, uint64_t layermask) = 0;
        virtual hypernode<I> solid(uint32_t depth) = 0;

        virtual hypernode<I> getchild(hypernode<I> hnode, uint32_t n) = 0;
        virtual uint64_t leafpart(I index, uint32_t part) = 0;

        hypernode<I> pyramid_down(hypernode<I> hnode) {

            if (hnode.depth <= 1) { return hnode; }
            if (hnode.index == 0) { return hypernode<I>(0, 1); }

            // Extract the pointer for the node and its children:
            kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr = ind2ptr_nonleaf(hnode.depth, hnode.index);
            kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr_tl = ind2ptr_nonleaf(hnode.depth-1, pptr->key.x[0]);
            kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr_tr = ind2ptr_nonleaf(hnode.depth-1, pptr->key.x[1]);
            kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr_bl = ind2ptr_nonleaf(hnode.depth-1, pptr->key.x[2]);
            kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr_br = ind2ptr_nonleaf(hnode.depth-1, pptr->key.x[3]);

            bool tl_good = (pptr_tl->key.x[0] == 0) && (pptr_tl->key.x[1] == 0) && (pptr_tl->key.x[2] == 0);
            bool tr_good = (pptr_tr->key.x[0] == 0) && (pptr_tr->key.x[1] == 0) && (pptr_tr->key.x[3] == 0);
            bool bl_good = (pptr_bl->key.x[0] == 0) && (pptr_bl->key.x[2] == 0) && (pptr_bl->key.x[3] == 0);
            bool br_good = (pptr_br->key.x[1] == 0) && (pptr_br->key.x[2] == 0) && (pptr_br->key.x[3] == 0);

            if (tl_good && tr_good && bl_good && br_good) {
                nicearray<I, 4> cc = {pptr_tl->key.x[3], pptr_tr->key.x[2], pptr_bl->key.x[1], pptr_br->key.x[0]};
                hypernode<I>  hncc = make_nonleaf_hn(hnode.depth-1, cc);
                // Do this recursively:
                return pyramid_down(hncc);
            } else {
                return hnode;
            }
        }

        virtual hypernode<I> demorton_recurse(std::map<uint64_t, uint64_t>::iterator &it,
                                              std::map<uint64_t, uint64_t>::iterator &pe,
                                              uint64_t lm, uint64_t upto, uint32_t depth) = 0;

        hypernode<I> demorton(std::map<uint64_t, uint64_t> &mmap, uint64_t lm) {
            std::map<uint64_t, uint64_t>::iterator it = mmap.begin();
            std::map<uint64_t, uint64_t>::iterator pe = mmap.end();
            return pyramid_down(demorton_recurse(it, pe, lm, 0, 31));
        }

        hypernode<I> demorton(const bitworld &bw, uint64_t lm) {
            std::map<uint64_t, uint64_t> mmap;
            bw.mortonmap(&mmap);
            return demorton(mmap, lm);
        }

        virtual void bitworld_recurse(hypernode<I> hnode, bitworld* bw, uint32_t layer, int32_t x, int32_t y) = 0;

        bitworld flatlayer(hypernode<I> hnode, uint32_t layer, int32_t x, int32_t y) {
            bitworld bw;
            bitworld_recurse(hnode, &bw, layer, x, y);
            return bw;
        }

        bitworld flatlayer(hypernode<I> hnode, uint32_t layer) {
            return flatlayer(hnode, layer, -(1 << (hnode.depth)), -(1 << (hnode.depth)));
        }

        virtual hypernode<I> shift_recurse(hypernode<I> hnode, uint64_t x, uint64_t y, uint64_t exponent,
                                            std::map<std::pair<I, uint32_t>, I> *memmap) = 0;

        hypernode<I> shift_recurse(hypernode<I> hnode, uint64_t x, uint64_t y, uint64_t exponent) {
            std::map<std::pair<I, uint32_t>, I> memmap;
            return shift_recurse(hnode, x, y, exponent, &memmap);
        }

        hypernode<I> shift_toroidal(hypernode<I> hnode, int64_t x, int64_t y, uint64_t exponent) {
            nicearray<I, 4> cc = {hnode.index, hnode.index, hnode.index, hnode.index};
            hypernode<I> xcc = make_nonleaf_hn(hnode.depth + 1, cc);

            int64_t sx = x;
            int64_t sy = y;
            uint64_t sz = exponent;

            if ((sx == 0) && (sy == 0)) { return hnode; }

            while (((sx & 1) == 0) && ((sy & 1) == 0)) {
                sx = sx / 2;
                sy = sy / 2;
                sz = sz + 1;
            }

            // We cast to unsigned integers, which is okay provided our
            // universe is no larger than (2 ^ 64)-by-(2 ^ 64):
            uint64_t ux = (uint64_t) (0 - sx);
            uint64_t uy = (uint64_t) (0 - sy);

            return shift_recurse(xcc, ux, uy, sz);
        }

        hypernode<I> pyramid_up(hypernode<I> hnode) {

            I z = 0;

            if (hnode.depth == 0) {
                nicearray<I, 4> cc = {z, z, z, hnode.index};
                hypernode<I> hnode2 = make_nonleaf_hn(hnode.depth + 1, cc);
                return shift_toroidal(hnode2, -1, -1, 3);
            } else {
                kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr = ind2ptr_nonleaf(hnode.depth, hnode.index);
                nicearray<I, 4> tl = {z, z, z, pptr->key.x[0]};
                nicearray<I, 4> tr = {z, z, pptr->key.x[1], z};
                nicearray<I, 4> bl = {z, pptr->key.x[2], z, z};
                nicearray<I, 4> br = {pptr->key.x[3], z, z, z};
                nicearray<I, 4> nc = {make_nonleaf(hnode.depth, tl),
                                      make_nonleaf(hnode.depth, tr),
                                      make_nonleaf(hnode.depth, bl),
                                      make_nonleaf(hnode.depth, br)};
                return make_nonleaf_hn(hnode.depth + 1, nc);
            }

        }

        hypernode<I> pyramid_up(hypernode<I> hnode_initial, uint32_t target_depth) {
            hypernode<I> hnode = hnode_initial;
            while (target_depth > hnode.depth) {
                // Do this iteratively:
                hnode = pyramid_up(hnode);
            }
            return hnode;
        }

        virtual hypernode<I> convolve_recurse(hypernode<I> lnode, hypernode<I> rnode, bool exclusive,
                                        std::map<std::pair<std::pair<I, I>, uint32_t>, I> *memmap1,
                                        std::map<std::pair<std::pair<I, I>, uint32_t>, I> *memmap2) = 0;

        hypernode<I> convolve_recurse(hypernode<I> lnode, hypernode<I> rnode, bool exclusive) {
            std::map<std::pair<std::pair<I, I>, uint32_t>, I> memmap1;
            std::map<std::pair<std::pair<I, I>, uint32_t>, I> memmap2;
            return convolve_recurse(lnode, rnode, exclusive, &memmap1, &memmap2);
        }

        hypernode<I> convolve_universe(hypernode<I> lnode, hypernode<I> rnode, bool exclusive) {
            hypernode<I> lx = lnode;
            hypernode<I> rx = rnode;
            while (lx.depth < rx.depth) { lx = pyramid_up(lx); }
            while (lx.depth > rx.depth) { rx = pyramid_up(rx); }
            hypernode<I> hnode = convolve_recurse(lx, rx, exclusive);
            hnode = pyramid_down(hnode);
            return hnode;
        }

        virtual uint64_t digest_recurse(hypernode<I> hnode,
                                        std::map<std::pair<I, uint32_t>, uint64_t> *memmap) = 0;

        virtual hypernode<I> boolean_recurse(hypernode<I> lnode, hypernode<I> rnode, int operation,
                                        std::map<std::pair<std::pair<I, I>, uint32_t>, I> *memmap) = 0;

        hypernode<I> boolean_recurse(hypernode<I> lnode, hypernode<I> rnode, int operation) {
            std::map<std::pair<std::pair<I, I>, uint32_t>, I> memmap;
            return boolean_recurse(lnode, rnode, operation, &memmap);
        }

        hypernode<I> boolean_universe(hypernode<I> lnode, hypernode<I> rnode, int operation) {
            hypernode<I> lx = lnode;
            hypernode<I> rx = rnode;
            while (lx.depth < rx.depth) { lx = pyramid_up(lx); }
            while (lx.depth > rx.depth) { rx = pyramid_up(rx); }
            hypernode<I> hnode = boolean_recurse(lx, rx, operation);
            hnode = pyramid_down(hnode);
            return hnode;
        }

        hypernode<I> fromplanes(std::vector<bitworld> planes) {
            /*
            * Convert a string from headerless RLE to a hypernode in the
            * current universe.
            */
            hypernode<I> hnode(0, 1);
            for (unsigned int i = 0; i < planes.size(); i++) {
                hnode = boolean_universe(hnode, demorton(planes[i], 1 << i), 1);
            }
            return hnode;
        }

        hypernode<I> fromrle(std::string s) {
            if (s != "") {
                return fromplanes(rle2vec(s));
            } else {
                return hypernode<I>(0, 1);
            }
        }

        virtual int rulestring_to_integer(std::string rulestring) = 0;
        virtual hypernode<I> iterate_recurse(hypernode<I> hnode, uint64_t mantissa, uint64_t exponent, int rule, int history) = 0;

        hypernode<I> advance(hypernode<I> hnode_initial, uint64_t mantissa, uint64_t exponent, int rule, bool history) {
            /*
            * Advance the universe by (mantissa * (2 ** exponent)) timesteps,
            * returning an index to the resulting hypernode. This resizes the
            * universe as necessary (without changing the 'origin', taken to
            * be the centre of the hypernode).
            */
            // std::cerr << "Exponent = " << exponent << " ; mantissa = " << mantissa << std::endl;
            int family = uli_get_family(rule) + history;
            hypernode<I> hnode = pyramid_up(pyramid_up(hnode_initial));
            hnode = pyramid_up(hnode, exponent + 2);
            hnode = iterate_recurse(hnode, mantissa, exponent, rule, family);
            hnode = pyramid_down(hnode);
            return hnode;
        }

        hypernode<I> advance(hypernode<I> hnode_initial, uint64_t mantissa, uint64_t exponent, std::string rule, bool history) {
            int ruleint = rulestring_to_integer(rule);
            if (ruleint == -1) {
                return hnode_initial;
            } else {
                return advance(hnode_initial, mantissa, exponent, ruleint, history);
            }
        }

        hypernode<I> advance(hypernode<I> hnode_initial, uint64_t mantissa, uint64_t exponent, std::string rule) {
            // std::cerr << mantissa << " * 2^" << exponent << std::endl;
            std::string newrule = rule;
            bool history = false;
            if (newrule.length() >= 8 && newrule.substr(newrule.length()-7) == "History") {
                newrule = newrule.substr(0, newrule.length()-7);
                history = true;
                if (newrule[newrule.length()-1] == '_') {
                    newrule = newrule.substr(0, newrule.length()-1);
                }
            }
            return advance(hnode_initial, mantissa, exponent, newrule, history);
        }

        hypernode<I> advance(hypernode<I> hnode_initial, std::string rule, uint64_t steps, uint64_t stepexp) {
            hypernode<I> hnode = hnode_initial;
            if (steps) {
                uint64_t numsteps = steps;
                uint64_t exponent = stepexp;

                while ((numsteps & 7) && exponent) {
                    numsteps = numsteps << 1;
                    exponent -= 1;
                }

                uint64_t vm = uli_valid_mantissa(rulestring_to_integer(rule));
                uint64_t mantissa = 8;
                while ((mantissa != 1) && ((numsteps % mantissa) || ((vm & (1 << mantissa)) == 0))) { mantissa -= 1; }
                if ((vm & (1 << mantissa)) == 0) {
                    std::cerr << "Rule " << rule << " cannot be iterated " << steps;
                    std::cerr << " x 2^" << stepexp << " generations." << std::endl;
                    exit(1);
                }

                uint64_t multiplier = numsteps / mantissa;

                while (multiplier) {
                    if (multiplier & 1) {
                        hnode = advance(hnode, mantissa, exponent, rule);
                    }
                    multiplier = multiplier >> 1;
                    exponent += 1;
                }
            } else {
                hnode = pyramid_down(hnode);
            }
            return hnode;
        }

        hypernode<I> advance(hypernode<I> hnode_initial, std::string rule, uint64_t steps) {
            return advance(hnode_initial, rule, steps, 0);
        }

        virtual uint64_t bound_recurse(hypernode<I> hnode, int direction, std::map<std::pair<I, uint32_t>, uint64_t> *memmap) = 0;

        uint64_t bound_recurse(hypernode<I> hnode, int direction) {
            std::map<std::pair<I, uint32_t>, uint64_t> memmap;
            return bound_recurse(hnode, direction, &memmap);
        }

        bool getbbox(hypernode<I> hnode, int64_t *bbox) {

            if (hnode.index == 0) { return false; }

            int64_t left = bound_recurse(hnode, 0);
            int64_t top = bound_recurse(hnode, 1);
            int64_t right = bound_recurse(hnode, 2);
            int64_t bottom = bound_recurse(hnode, 3);

            bbox[0] = left - (8 << hnode.depth);
            bbox[1] = top - (8 << hnode.depth);
            bbox[2] = 1 + right - left;
            bbox[3] = 1 + bottom - top;

            return true;
        }

        uint64_t digest_universe(hypernode<I> hnode) {
            std::map<std::pair<I, uint32_t>, uint64_t> memmap;
            uint64_t h = digest_recurse(hnode, &memmap);
            h += (bound_recurse(hnode, 0) * 5);
            h += (bound_recurse(hnode, 1) * 17);
            h += (bound_recurse(hnode, 2) * 257);
            h += (bound_recurse(hnode, 3) * 65537);
            return h;
        }

        virtual hypernode<I> tensor_recurse(hypernode<I> hnode, lifetree_abstract<I> *lab, uint32_t delta, std::vector<I> &v,
                                    std::map<std::pair<I, uint32_t>, I> *memmap) = 0;

        hypernode<I> tensor_recurse(hypernode<I> hnode, lifetree_abstract<I> *lab, uint32_t delta, std::vector<I> &v) {
            std::map<std::pair<I, uint32_t>, I> memmap;
            return tensor_recurse(hnode, lab, delta, v, &memmap);
        }

        virtual hypernode<I> copy_recurse(hypernode<I> hnode, lifetree_abstract<I> *lab, std::map<std::pair<I, uint32_t>, I> *memmap) = 0;

        hypernode<I> copy_recurse(hypernode<I> hnode, lifetree_abstract<I> *lab) {
            std::map<std::pair<I, uint32_t>, I> memmap;
            return copy_recurse(hnode, lab, &memmap);
        }

        virtual hypernode<I> brand_recurse(hypernode<I> hnode, std::map<std::pair<I, uint32_t>, I> *memmap) = 0;

        hypernode<I> brand_recurse(hypernode<I> hnode) {
            std::map<std::pair<I, uint32_t>, I> memmap;
            return brand_recurse(hnode, &memmap);
        }

        virtual hypernode<I> transform_recurse(hypernode<I> hnode, uint8_t perm, std::map<std::pair<I, uint32_t>, I> *memmap) = 0;

        hypernode<I> transform_recurse(hypernode<I> hnode, uint8_t perm) {
            std::map<std::pair<I, uint32_t>, I> memmap;
            return transform_recurse(hnode, perm, &memmap);
        }

        hypernode<I> shift_universe(hypernode<I> hnode_initial, int64_t x, int64_t y, uint64_t exponent) {
            hypernode<I> hnode = hnode_initial;
            if ((x != 0) || (y != 0)) {
                int64_t absx = (x < 0) ? (-x) : x;
                int64_t absy = (y < 0) ? (-y) : y;
                uint64_t diameter = (absx > absy) ? absx : absy;
                uint64_t lzcount = __builtin_clzll(diameter);
                hnode = pyramid_up(hnode_initial, (64 - lzcount) + exponent);
                hnode = pyramid_up(hnode);
                hnode = shift_toroidal(hnode, x, y, exponent);
            }
            hnode = pyramid_down(hnode);
            return hnode;
        }

        hypernode<I> shift_universe(hypernode<I> hnode_initial, int64_t x, int64_t y) {
            return shift_universe(hnode_initial, x, y, 0);
        }

        hypernode<I> rectangle(uint64_t width, uint64_t height) {
            if (width == 0 || height == 0) {
                return hypernode<I>(0, 1);
            } else {
                uint64_t diameter = (width > height) ? width : height;
                uint64_t lzcount = __builtin_clzll(diameter);
                hypernode<I> hnode = solid((lzcount < 60) ? (60 - lzcount) : 1);
                I z = 0;
                nicearray<I, 4> cc = {z, z, z, hnode.index};
                hnode = make_nonleaf_hn(hnode.depth + 1, cc);
                hnode = boolean_universe(hnode, shift_universe(hnode, width, 0, 0), 3);
                hnode = boolean_universe(hnode, shift_universe(hnode, 0, height, 0), 3);
                return hnode;
            }
        }

        hypernode<I> rectangle(int64_t x, int64_t y, uint64_t width, uint64_t height) {
            /*
            * Return a hnode containing a solid rectangle with top-left corner
            * (x, y) and dimensions width * height.
            */
            hypernode<I> hnode = rectangle(width, height);
            hnode = shift_universe(hnode, x, y, 0);
            return hnode;
        }

        hypernode<I> transform_and_shift(hypernode<I> hnode, uint8_t perm, int64_t x, int64_t y) {
            /*
            * Apply an isometry of the plane which sends (0, 0) to (x, y) with
            * an optional rotation and/or reflection.
            */
            int64_t nx = (perm & 128) ? x : (x + 1);
            int64_t ny = (perm & 64) ? y : (y + 1);
            hypernode<I> hn = (perm == 228) ? hnode : transform_recurse(hnode, perm);
            return shift_universe(hn, nx, ny, 0);
        }

        virtual hypernode<I> read_macrocell(std::istream &instream, std::map<uint64_t, uint64_t> *lmap, std::string &rule) = 0;

        hypernode<I> load_macrocell(std::string filename, std::string &rule) {
            std::ifstream f(filename);
            std::map<uint64_t, uint64_t> lmap;
            lmap[0] = 0;
            lmap[1] = 3;
            lmap[2] = 2;
            lmap[3] = 11;
            lmap[4] = 10;
            lmap[5] = 15;
            return read_macrocell(f, &lmap, rule);
        }

        hypernode<I> load_macrocell(std::string filename, std::string &rule, int64_t x, int64_t y) {
            return shift_universe(load_macrocell(filename, rule), x, y);
        }

        hypernode<I> pattern_match(hypernode<I> u, hypernode<I> c0, hypernode<I> c1) {
            hypernode<I> v = pyramid_up(pyramid_up(pyramid_up(u, c0.depth), c1.depth));
            hypernode<I> sol = solid(v.depth);
            hypernode<I> sol2 = solid(v.depth-1);
            hypernode<I> rc0 = pyramid_up(transform_recurse(c0, 27), v.depth);
            hypernode<I> rc1 = pyramid_up(transform_recurse(c1, 27), v.depth);
            hypernode<I> match0 = convolve_recurse(v, rc0, false);
            hypernode<I> match1 = convolve_recurse(boolean_recurse(sol, v, 3), rc1, false);
            hypernode<I> badcells = boolean_recurse(match0, match1, 1);
            hypernode<I> goodcells = boolean_recurse(pyramid_up(pyramid_up(sol2)), badcells, 3);
            return pyramid_down(shift_toroidal(brand_recurse(goodcells), 1, 1, 0));
        }

    };
}
